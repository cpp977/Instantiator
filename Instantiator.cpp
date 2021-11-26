#include <iostream>
#include <ranges>
#include <sstream>
#include <unordered_set>

#include "termcolor/termcolor.hpp"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/AST/ASTImporter.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"

#include "Injection.hpp"

#include "Callbacks/DeleteInstantiations.hpp"
#include "Callbacks/GetNeededInstantiations.hpp"
#include "Callbacks/InjectInstantiation.hpp"
#include "Matcher/Matcher.hpp"
//
// Command line options
//
// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory InstantiatorOptions("Instantiator options");

static llvm::cl::opt<bool> Clean("clean", llvm::cl::desc("Delete all explicit instantiations."), llvm::cl::cat(InstantiatorOptions));
static llvm::cl::list<std::string> IgnorePatterns("i",
                                                  llvm::cl::desc("List of namespaces which should be ignored."),
                                                  llvm::cl::desc("<pattern>"),
                                                  llvm::cl::cat(InstantiatorOptions));

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static llvm::cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char** argv)
{
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, InstantiatorOptions, llvm::cl::OneOrMore);
    if(!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    clang::tooling::CommonOptionsParser& OptionsParser = ExpectedParser.get();

    if(IgnorePatterns.size() == 0) { IgnorePatterns.push_back("std"); }

    clang::ast_matchers::internal::Matcher<clang::NamedDecl> nameMatcher = clang::ast_matchers::matchesName(IgnorePatterns[0] + "::");
    for(auto it = IgnorePatterns.begin() + 1; it != IgnorePatterns.end(); it++) {
        nameMatcher = clang::ast_matchers::anyOf(nameMatcher, clang::ast_matchers::matchesName(*it + "::"));
    }

    clang::ast_matchers::DeclarationMatcher TemplateInstantiationMatcher = TemplInstWithoutDef(nameMatcher);

    clang::ast_matchers::DeclarationMatcher FunctionDefMatcher = FuncWithDef(nameMatcher);

    // clang::ast_matchers::functionDecl(clang::ast_matchers::isDefinition(), clang::ast_matchers::unless(nameMatcher)).bind("func_definition");

    std::vector<std::string> main_and_injection_files = OptionsParser.getCompilations().getAllFiles();

    std::cout << "source file from command line: " << OptionsParser.getSourcePathList()[0] << std::endl;
    for(auto it = main_and_injection_files.begin(); it != main_and_injection_files.end(); it++) {
        if(it->find(OptionsParser.getSourcePathList()[0]) != std::string::npos) { std::iter_swap(main_and_injection_files.begin(), it); }
    }
    std::cout << "source files from json" << std::endl;
    for(const auto& source : main_and_injection_files) { std::cout << '\t' << "-- " << source << std::endl; }
    llvm::ArrayRef<std::string> sources(main_and_injection_files.data(), main_and_injection_files.size());

    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), sources);

    std::vector<std::unique_ptr<clang::ASTUnit>> allASTs;
    std::vector<Injection> toDoList;
    [[maybe_unused]] int success = Tool.buildASTs(allASTs);
    std::map<std::string, std::size_t> file2AST;
    for(std::size_t i = 0; i < allASTs.size(); i++) { file2AST.insert(std::make_pair(main_and_injection_files[i], i)); }
    std::cout << "Parsed ASTs done." << std::endl << std::endl;

    if(Clean) {
        for(std::size_t i = 0; i < allASTs.size(); i++) {
            std::cout << "Clean all explicit instantiations in " << allASTs[i]->getMainFileName().str() << "." << std::endl;
            DeleteInstantiations Deleter;
            clang::Rewriter rewriter(allASTs[i]->getSourceManager(), allASTs[i]->getLangOpts());
            Deleter.rewriter = &rewriter;
            clang::ast_matchers::MatchFinder Inst_Finder;
            Inst_Finder.addMatcher(/*Matcher*/ TemplInst(nameMatcher), /*Callback*/ &Deleter);
            Inst_Finder.matchAST(allASTs[i]->getASTContext());
            rewriter.overwriteChangedFiles();
            std::cout << "Done." << std::endl;
        }
        return 0;
    }

    GetNeededInstantiations Getter;

    Getter.toDoList = &toDoList;
    clang::ast_matchers::MatchFinder Finder;
    Finder.addMatcher(/*Matcher*/ TemplateInstantiationMatcher, /*Callback*/ &Getter);

    // match in current main file.
    std::unordered_set<std::string> workList;
    workList.insert(main_and_injection_files[0]);

    while(workList.size() > 0) {
        auto copyOf_workList = workList;
        for(const auto& item : copyOf_workList) {
            workList.erase(item);
            Finder.matchAST(allASTs[file2AST[item]]->getASTContext());
            std::cout << termcolor::bold << termcolor::blue << "Run on file " << item << " produced the following ToDo-List:" << termcolor::reset
                      << std::endl;
            for(const auto& toDo : toDoList) { std::cout << '\t' << toDo << std::endl; }
            for(const auto& file_for_search : main_and_injection_files) {
                std::cout << termcolor::green << "Search in AST of file " << file_for_search << termcolor::reset << std::endl;
                clang::Rewriter rewriter(allASTs[file2AST[file_for_search]]->getSourceManager(), allASTs[file2AST[file_for_search]]->getLangOpts());
                clang::ast_matchers::MatchFinder FuncFinder;
                InjectInstantiation instantiator;
                instantiator.toDoList = &toDoList;
                instantiator.rewriter = &rewriter;
                FuncFinder.addMatcher(/*Matcher*/ FunctionDefMatcher, /*Callback*/ &instantiator);
                try {
                    FuncFinder.matchAST(allASTs[file2AST[file_for_search]]->getASTContext());
                    std::cout << termcolor::green << "Called AST match function" << termcolor::reset << std::endl;
                } catch(const std::exception& e) {
                    std::cout << " a standard exception was caught during AST parsing, with message '" << e.what() << "'\n";
                }
                try {
                    rewriter.overwriteChangedFiles();
                    std::cout << termcolor::green << "Called rewriter" << termcolor::reset << std::endl;
                } catch(...) {
                    std::cout << " a exception was caught during rewrite step" << std::endl;
                }
                bool HAS_INJECTED_INTANTIATION = rewriter.buffer_begin() != rewriter.buffer_end();
                if(HAS_INJECTED_INTANTIATION) {
                    workList.insert(file_for_search);
                    auto PCHContainerOps = std::make_shared<clang::PCHContainerOperations>();
                    bool AST_NOT_UPDATED = allASTs[file2AST[file_for_search]]->Reparse(PCHContainerOps);
                    if(AST_NOT_UPDATED) { std::cerr << "Error while reparsing the AST" << std::endl; }
                }
                std::cout << std::endl;
            }
        }
    }
    std::cout << termcolor::bold << termcolor::red << "#toDos that are left: " << toDoList.size() << termcolor::reset << std::endl;
    for(const auto& toDo : toDoList) { std::cout << '\t' << termcolor::underline << toDo << termcolor::reset << std::endl; }
}
