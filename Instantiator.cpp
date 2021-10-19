#include <iostream>
#include <ranges>
#include <sstream>
#include <unordered_set>

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

#include "Callbacks/GetNeededInstantiations.hpp"
#include "Callbacks/InjectInstantiation.hpp"

using namespace clang::ast_matchers;

clang::ast_matchers::DeclarationMatcher TemplateInstantiationMatcher =
    functionDecl(isTemplateInstantiation(), unless(isDefinition()), unless(matchesName("std::")), unless(matchesName("__gnu_cxx::")))
        .bind("templ_func_instantation");

clang::ast_matchers::DeclarationMatcher FunctionDefMatcher = functionDecl(isDefinition()).bind("func_definition");

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("Instantiator options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static llvm::cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char** argv)
{
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::OneOrMore);
    if(!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    clang::tooling::CommonOptionsParser& OptionsParser = ExpectedParser.get();

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

    GetNeededInstantiations Getter;

    Getter.toDoList = &toDoList;
    clang::ast_matchers::MatchFinder Finder;
    Finder.addMatcher(/*Matcher*/ TemplateInstantiationMatcher, /*Callback*/ &Getter);

    // match in current main file.
    std::unordered_set<std::string> work_list;
    work_list.insert(main_and_injection_files[0]);

    while(work_list.size() > 0) {
        auto copyOf_work_list = work_list;
        for(const auto& item : copyOf_work_list) {
            work_list.erase(item);
            Finder.matchAST(allASTs[file2AST[item]]->getASTContext());
            std::cout << "*****************************************************************************************************************"
                      << std::endl;
            std::cout << "Run on file " << item << " produced the following ToDo-List:" << std::endl;
            std::cout << "*****************************************************************************************************************"
                      << std::endl;
            for(const auto& toDo : toDoList) { std::cout << toDo << std::endl; }
            std::cout << std::endl;
            for(const auto& file_for_search : main_and_injection_files) {
                if(file_for_search == item) { continue; }
                std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                          << std::endl;
                std::cout << "Search in AST of file " << file_for_search << "=="
                          << allASTs[file2AST[file_for_search]]
                                 ->getSourceManager()
                                 .getFileEntryForID(allASTs[file2AST[file_for_search]]->getSourceManager().getMainFileID())
                                 ->getName()
                                 .str()
                          << std::endl;
                std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                          << std::endl;
                clang::Rewriter rewriter(allASTs[file2AST[file_for_search]]->getSourceManager(), allASTs[file2AST[file_for_search]]->getLangOpts());
                clang::ast_matchers::MatchFinder FuncFinder;
                InjectInstantiation instantiator;
                instantiator.toDoList = &toDoList;
                instantiator.rewriter = &rewriter;
                FuncFinder.addMatcher(/*Matcher*/ FunctionDefMatcher, /*Callback*/ &instantiator);
                FuncFinder.matchAST(allASTs[file2AST[file_for_search]]->getASTContext());
                std::cout << std::endl;
                rewriter.overwriteChangedFiles();
                bool HAS_INJECTED_INTANTIATION = rewriter.buffer_begin() != rewriter.buffer_end();
                if(HAS_INJECTED_INTANTIATION) {
                    work_list.insert(file_for_search);
                    auto PCHContainerOps = std::make_shared<clang::PCHContainerOperations>();
                    bool AST_NOT_UPDATED = allASTs[file2AST[file_for_search]]->Reparse(PCHContainerOps);
                    if(AST_NOT_UPDATED) { std::cerr << "Error while reparsinf the AST" << std::endl; }
                }
            }
        }
    }
    std::cout << "#toDos that are left: " << toDoList.size() << std::endl;
}
