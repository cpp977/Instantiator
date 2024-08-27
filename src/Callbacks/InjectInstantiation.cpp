#include "Callbacks/InjectInstantiation.hpp"

#include <filesystem>
#include <iostream>

#include "spdlog/spdlog.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"

#include "Injection.hpp"
#include "Template.hpp"

void InjectInstantiation::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    pp.PrintInjectedClassNameWithArguments = true;
    pp.PrintCanonicalTypes = true;
    pp.SuppressDefaultTemplateArgs = true;
    pp.FullyQualifiedName = true;
    pp.SuppressScope = false;
    // pp.UsePreferredNames = true;

    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("func_definition")) {
        spdlog::debug("Processing memfunc {}", MFS->getNameAsString());
        spdlog::debug("TI={}, CTI={}", MFS->isTemplateInstantiation(), (MFS->getParent()->getMemberSpecializationInfo() != nullptr));
        if(MFS->isTemplateInstantiation() or (MFS->getParent()->getMemberSpecializationInfo() != nullptr)) {
            if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
                if(MSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }
            auto candidate = Injection::createFromMFS(MFS, pp);

            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            spdlog::debug("Check if the instantiation is already present.");
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;

                if(candidate.match(toDo)) {
                    spdlog::debug("Erasing element from toDolist:\n {} because of: {}", toDo, candidate);
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantiation is needed. inject the
            // instantation in the Rewriter.
            spdlog::debug("Check for match.");
            auto candidate = Template::createFromMFS(MFS, pp);
            spdlog::debug("Processing candidate: {}", candidate);
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                spdlog::debug("CHecking toDo entry: {}", toDo);
                if(candidate.isTemplateFor(toDo)) {
                    if(invasive) {
                        rewriter->InsertText(MFS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    } else {
                        spdlog::debug("Match!!! Call the rewriter and delete entry from toDoList.");
                        auto sc = MFS->getBodyRBrace().getLocWithOffset(1);
                        auto fid = rewriter->getSourceMgr().getFileID(sc);
                        auto fileentry = rewriter->getSourceMgr().getFileEntryRefForID(fid);
                        auto fname_ = rewriter->getSourceMgr().getFileManager().getCanonicalName(*fileentry);
                        std::string fname(fname_.data(), fname_.size());
                        auto new_name = std::filesystem::path(fname);
                        new_name.replace_extension("gen.cpp");
                        auto& sm = rewriter->getSourceMgr();
                        auto& fm = sm.getFileManager();
                        auto new_name_str = new_name.string();
                        llvm::StringRef gen_name(new_name_str);
                        auto file_ref = fm.getFileRef(gen_name, true);
                        auto new_fid = sm.getOrCreateFileID(*file_ref, clang::SrcMgr::C_User);
                        auto new_loc = sm.getLocForEndOfFile(new_fid);
                        rewriter->InsertText(new_loc, llvm::StringRef(it->getInstantiation()), true, true);
                    }
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("func_definition")) {
        spdlog::debug("Processing func {}", FS->getNameAsString());
        if(FS->isTemplateInstantiation()) {
            if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }
            auto candidate = Injection::createFromFS(FS, pp);
            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            spdlog::debug("Check if the instantiation is already present.");
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(candidate.match(toDo)) {
                    spdlog::debug("Erase from toDolist.");
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantation is needed. inject the
            // instantation in the Rewriter.
            spdlog::debug("Check if the correct prototype is present for explicit instantiation.");
            auto candidate = Template::createFromFS(FS, pp);
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(candidate.isTemplateFor(toDo)) {
                    spdlog::debug("Match!!! Call the rewriter and delete entry from toDoList.");
                    spdlog::debug("Injection: {}", it->getInstantiation());
                    if(invasive) {
                        rewriter->InsertText(FS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    } else {
                        auto sc = FS->getBodyRBrace().getLocWithOffset(1);
                        auto fid = rewriter->getSourceMgr().getFileID(sc);
                        auto fileentry = rewriter->getSourceMgr().getFileEntryRefForID(fid);
                        auto fname_ = rewriter->getSourceMgr().getFileManager().getCanonicalName(*fileentry);
                        std::string fname(fname_.data(), fname_.size());
                        auto new_name = std::filesystem::path(fname);
                        new_name.replace_extension("gen.cpp");
                        std::cout << "Injecting in file " << new_name << std::endl;
                        auto& sm = rewriter->getSourceMgr();
                        auto& fm = sm.getFileManager();
                        auto new_name_str = new_name.string();
                        llvm::StringRef gen_name(new_name_str);
                        auto file_ref = fm.getFileRef(gen_name, true);
                        auto new_fid = sm.getOrCreateFileID(*file_ref, clang::SrcMgr::C_User);
                        auto new_loc = sm.getLocForEndOfFile(new_fid);
                        rewriter->InsertText(new_loc, llvm::StringRef(it->getInstantiation()), true, true);
                    }
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        }
    }
}
