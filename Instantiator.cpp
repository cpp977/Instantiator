// Declares clang::SyntaxOnlyAction.
#include <iostream>
#include <ranges>
#include <sstream>

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

using namespace clang;
using namespace clang::ast_matchers;

DeclarationMatcher TemplateInstantiationMatcher =
    functionDecl(isTemplateInstantiation(), unless(isDefinition()), unless(matchesName("std::")), unless(matchesName("__gnu_cxx::")))
        .bind("templ_func_instantation");

DeclarationMatcher FunctionDefMatcher = functionDecl(isDefinition()).bind("func_definition");

using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("Instantiator options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char** argv)
{
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::OneOrMore);
    if(!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();

    std::vector<std::string> main_and_injection_files = OptionsParser.getCompilations().getAllFiles();

    std::cout << "source file from command line: " << OptionsParser.getSourcePathList()[0] << std::endl;
    for(auto it = main_and_injection_files.begin(); it != main_and_injection_files.end(); it++) {
        if(it->find(OptionsParser.getSourcePathList()[0]) != std::string::npos) { std::iter_swap(main_and_injection_files.begin(), it); }
    }
    std::cout << "source files from json" << std::endl;
    for(const auto& source : main_and_injection_files) { std::cout << '\t' << "-- " << source << std::endl; }
    llvm::ArrayRef<std::string> sources(main_and_injection_files.data(), main_and_injection_files.size());

    ClangTool Tool(OptionsParser.getCompilations(), sources);

    std::vector<std::unique_ptr<ASTUnit>> allASTs;
    std::vector<Injection> toDoList;
    [[maybe_unused]] int success = Tool.buildASTs(allASTs);
    // TemplatePrinter Printer;
    GetNeededInstantiations Getter;

    Getter.toDoList = &toDoList;
    MatchFinder Finder;
    Finder.addMatcher(TemplateInstantiationMatcher, &Getter);

    // match in current main file.
    Finder.matchAST(allASTs[0]->getASTContext());
    std::cout << "**************************************************" << std::endl;
    std::cout << "Run on main file produced the following ToDo-List:" << std::endl;
    std::cout << "**************************************************" << std::endl;
    for(const auto& toDo : toDoList) { std::cout << toDo << std::endl; }
    std::cout << std::endl;
    for(std::size_t i = 1; i < allASTs.size(); i++) {
        // const auto SM = allASTs[i]->getSourceManager();
        std::cout << "*****************************************************************************************************************" << std::endl;
        std::cout << "Search in AST of file "
                  << allASTs[i]->getSourceManager().getFileEntryForID(allASTs[i]->getSourceManager().getMainFileID())->getName().str() << std::endl;
        std::cout << "*****************************************************************************************************************" << std::endl;
        Rewriter rewriter(allASTs[i]->getSourceManager(), allASTs[i]->getLangOpts());
        MatchFinder FuncFinder;
        InjectInstantiation instantiator;
        instantiator.toDoList = &toDoList;
        instantiator.rewriter = &rewriter;
        FuncFinder.addMatcher(/*Matcher*/ FunctionDefMatcher, /*Callback*/ &instantiator);
        FuncFinder.matchAST(allASTs[i]->getASTContext());
        std::cout << std::endl;
        rewriter.overwriteChangedFiles();
    }
    std::cout << "#toDos that are left: " << toDoList.size() << std::endl;
}
