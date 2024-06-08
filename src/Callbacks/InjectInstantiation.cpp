#include "Callbacks/InjectInstantiation.hpp"

#include "Injection.hpp"
#include "Template.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"

#include <filesystem>
#include <iostream>

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
        // std::cout << std::boolalpha << "TI=" << MFS->isTemplateInstantiation()
        //           << ", CTI=" << (MFS->getParent()->getMemberSpecializationInfo() != nullptr) << std::endl;
        // std::cout << "Processing memfunc " << MFS->getNameAsString() << std::endl;
        // llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
        if(MFS->isTemplateInstantiation() or (MFS->getParent()->getMemberSpecializationInfo() != nullptr)) {
            if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
                if(MSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }
            auto candidate = Injection::createFromMFS(MFS, pp);

            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present." << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;

                if(candidate.match(toDo)) {
                    // std::cout << "Erasing element from toDolist: " << std::endl
                    //           << toDo << std::endl
                    //           << "because of:" << std::endl
                    //           << candidate << std::endl;
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantation is needed. inject the
            // instantation in the Rewriter.
            // std::cout << "Check for match." << std::endl;
            auto candidate = Template::createFromMFS(MFS, pp);
            // std::cout << "Processing candidate: " << candidate << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                // std::cout << "Checking toDo entry: " << toDo << std::endl;
                if(candidate.isTemplateFor(toDo)) {
                    if(invasive) {
                        rewriter->InsertText(MFS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    } else {
                        // std::cout << "Match!!! Call the rewriter and delete entry from toDoList." << std::endl;
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
        // std::cout << "Processing func " << FS->getNameAsString() << std::endl;
        if(FS->isTemplateInstantiation()) {
            if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }
            auto candidate = Injection::createFromFS(FS, pp);
            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present."
            //           << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(candidate.match(toDo)) {
                    // std::cout << "Erase from toDolist." << std::endl;
                    it = toDoList->erase(it);
                } else {
                    // std::cout << std::boolalpha << "FT=" << params_match
                    //           << ", FTP=" << func_tparam_match << std::endl;
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantation is needed. inject the
            // instantation in the Rewriter.
            // std::cout << "Check if the correct prototype is present for
            // explicit "
            //              "instantiation."
            //           << std::endl;
            auto candidate = Template::createFromFS(FS, pp);
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(candidate.isTemplateFor(toDo)) {
                    // std::cout
                    //     << "Match!!! Call the rewriter and delete entry from
                    //     toDoList."
                    //     << std::endl;
                    // std::cout << "Injection: " << it->getInstantiation() << std::endl;
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
