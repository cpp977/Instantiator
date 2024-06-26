#include "ASTCreation.hpp"
#include "Callbacks/DeleteInstantiations.hpp"
#include "Callbacks/GetNeededInstantiations.hpp"
#include "Callbacks/InjectInstantiation.hpp"
#include "IO/ProgressBar.hpp"
#include "Injection.hpp"
#include "Matcher/Matcher.hpp"
#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

#include "clang/AST/ASTImporter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>
#include <unordered_set>

//
// Command line options
//
static llvm::cl::OptionCategory InstantiatorOptions("Instantiator options");

static llvm::cl::opt<bool> Invasive("invasive",
                                    llvm::cl::desc("Inject instantiations invasively. (Directly after function definition.)"),
                                    llvm::cl::cat(InstantiatorOptions));

static llvm::cl::opt<bool> Clean("clean", llvm::cl::desc("Delete all explicit instantiations."), llvm::cl::cat(InstantiatorOptions));
static llvm::cl::list<std::string>
    IgnorePatterns("ignore",
                   llvm::cl::desc("List of namespaces which should be ignored. Several namespaces can be added by multiple --ignore calls."),
                   llvm::cl::value_desc("namespace"),
                   llvm::cl::cat(InstantiatorOptions));
static llvm::cl::alias IgnorePatternsA("i", llvm::cl::desc("Alias for --ignore"), llvm::cl::aliasopt(IgnorePatterns));

static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

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

    std::error_code tmp_create_error;
    auto tmpdir = std::filesystem::temp_directory_path(tmp_create_error);
    if(tmp_create_error) {
        std::cerr << "Error (" << tmp_create_error.value() << ") while getting temporary directory:\n" << tmp_create_error.message() << std::endl;
        return 1;
    }

    std::vector<std::filesystem::path> main_and_injection_files;
    for(const auto& file : OptionsParser.getCompilations().getAllFiles()) { main_and_injection_files.push_back(std::filesystem::path(file)); }

    // std::cout << "source files from json" << std::endl;
    // for(const auto& source : main_and_injection_files) { std::cout << '\t' << "-- " << source << std::endl; }
    llvm::ArrayRef<std::filesystem::path> sources(main_and_injection_files.data(), main_and_injection_files.size());

    if(Clean) {

        ProgressBar deletion_bar(sources.size());
        // indicators::ProgressBar deletion_bar{indicators::option::BarWidth{50},
        //                                      indicators::option::Start{"["},
        //                                      indicators::option::Fill{"■"},
        //                                      indicators::option::Lead{"■"},
        //                                      indicators::option::Remainder{"-"},
        //                                      indicators::option::End{" ]"},
        //                                      indicators::option::PrefixText{"Deleting instantiations: "},
        //                                      indicators::option::ForegroundColor{indicators::Color::red},
        //                                      indicators::option::ShowPercentage{true},
        //                                      indicators::option::MaxProgress{static_cast<int>(sources.size())},
        //                                      indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
        for(std::size_t i = 0; i < sources.size(); i++) {
            // deletion_bar.message = sources[i].string();
            bool HAS_INJECTED_INSTANTIATION = true;
            while(HAS_INJECTED_INSTANTIATION) {
                // deletion_bar.set_option(indicators::option::PostfixText{"Processing: " + sources[i].string()});
                if(not Invasive) {
                    auto gen_file = sources[i];
                    gen_file.replace_extension("gen.cpp");
                    std::ofstream f(gen_file, std::ios::out | std::ios::trunc);
                    f.close();
                    HAS_INJECTED_INSTANTIATION = false;
                } else {
                    std::unique_ptr<clang::ASTUnit> AST;
                    parseOrLoadAST(AST, OptionsParser.getCompilations(), sources[i], tmpdir);
                    clang::Rewriter rewriter(AST->getSourceManager(), AST->getLangOpts());
                    DeleteInstantiations Deleter;
                    Deleter.rewriter = &rewriter;
                    clang::ast_matchers::MatchFinder Inst_Finder;
                    Inst_Finder.addMatcher(/*Matcher*/ TemplInst(nameMatcher), /*Callback*/ &Deleter);
                    Inst_Finder.matchAST(AST->getASTContext());
                    rewriter.overwriteChangedFiles();
                    HAS_INJECTED_INSTANTIATION = rewriter.buffer_begin() != rewriter.buffer_end();
                }
            }
            deletion_bar.step();
        }
        fmt::print("\n");
        return 0;
    }

    GetNeededInstantiations Getter;

    std::vector<Injection> toDoList;
    Getter.toDoList = &toDoList;
    clang::ast_matchers::MatchFinder Finder;
    Finder.addMatcher(/*Matcher*/ TemplateInstantiationMatcher, /*Callback*/ &Getter);

    // match in current main file.
    std::unordered_set<std::string> workList;
    // workList.insert(main_and_injection_files[0]);
    workList.insert(OptionsParser.getSourcePathList()[0]);
    // indicators::IndeterminateProgressBar outer_bar{indicators::option::BarWidth{40},
    //                                                indicators::option::Start{"["},
    //                                                indicators::option::Fill{"·"},
    //                                                indicators::option::Lead{"<==>"},
    //                                                indicators::option::End{"]"},
    //                                                indicators::option::PrefixText{"Main loop"},
    //                                                indicators::option::ForegroundColor{indicators::Color::yellow},
    //                                                indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

    while(workList.size() > 0) {
        auto copyOf_workList = workList;
        // ProgressBar outer_bar{copyOf_workList.size()};
        for(const auto& item : copyOf_workList) {
            // std::cout << "Processing file " << item << std::endl;
            // outer_bar.set_option(indicators::option::PostfixText{"Scanning: " + item});
            workList.erase(item);
            std::unique_ptr<clang::ASTUnit> source_AST;
            parseOrLoadAST(source_AST, OptionsParser.getCompilations(), item, tmpdir);
            // std::cout << "Got AST" << std::endl;
            Finder.matchAST(source_AST->getASTContext());
            // std::cout << termcolor::bold << "Run on file " << item << " produced " << toDoList.size() << " ToDos" << termcolor::reset << std::endl;
            // for(const auto& toDo : toDoList) { std::cout << '\t' << toDo << std::endl; }
            ProgressBar inner_bar(toDoList.size());
            // indicators::ProgressBar inner_bar{indicators::option::BarWidth{50},
            //                                   indicators::option::Start{"["},
            //                                   indicators::option::Fill{"■"},
            //                                   indicators::option::Lead{"■"},
            //                                   indicators::option::Remainder{"-"},
            //                                   indicators::option::End{" ]"},
            //                                   indicators::option::PrefixText{"Checking for places to inject: "},
            //                                   indicators::option::ForegroundColor{indicators::Color::green},
            //                                   indicators::option::ShowPercentage{true},
            //                                   indicators::option::MaxProgress{static_cast<int>(main_and_injection_files.size())},
            //                                   indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
            // indicators::show_console_cursor(false);

            for(const auto& file_for_search : main_and_injection_files) {
                if(toDoList.size() == 0) {
                    // inner_bar.set_option(indicators::option::PostfixText{"ToDoList became empty. "});
                    inner_bar.update(inner_bar.numTasks());
                    break;
                }
                std::unique_ptr<clang::ASTUnit> target_AST;
                parseOrLoadAST(target_AST, OptionsParser.getCompilations(), file_for_search, tmpdir);
                // std::cout << termcolor::green << "Search in AST of file " << file_for_search << termcolor::reset << std::endl;
                clang::Rewriter rewriter(target_AST->getSourceManager(), target_AST->getLangOpts());
                clang::ast_matchers::MatchFinder FuncFinder;
                InjectInstantiation instantiator;
                instantiator.toDoList = &toDoList;
                instantiator.rewriter = &rewriter;
                instantiator.invasive = Invasive;
                FuncFinder.addMatcher(/*Matcher*/ FunctionDefMatcher, /*Callback*/ &instantiator);
                FuncFinder.matchAST(target_AST->getASTContext());
                // std::cout << termcolor::green << "Called AST match function" << termcolor::reset << std::endl;
                rewriter.overwriteChangedFiles();
                // std::cout << termcolor::green << "Called rewriter" << termcolor::reset << std::endl;
                bool HAS_INJECTED_INTANTIATION = rewriter.buffer_begin() != rewriter.buffer_end();
                // std::cout << "HAS_INJECTED=" << std::boolalpha << HAS_INJECTED_INTANTIATION << std::endl;
                if(HAS_INJECTED_INTANTIATION) { workList.insert(file_for_search); }
                inner_bar.step();
                // std::cout << std::endl;
            }
            fmt::print("\n");
            // outer_bar.tick();
        }
    }
    spdlog::info("#toDos that are left: {}", toDoList.size());
}
