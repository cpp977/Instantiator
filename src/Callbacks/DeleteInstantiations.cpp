// #include <iostream>

#include "Callbacks/DeleteInstantiations.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"

#include "clang/Rewrite/Core/Rewriter.h"

void DeleteInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    pp.PrintCanonicalTypes = 1;

    clang::SourceManager& sm = Result.Context->getSourceManager();

    clang::Rewriter::RewriteOptions opts;
    opts.IncludeInsertsAtBeginOfRange = true;
    opts.IncludeInsertsAtEndOfRange = true;
    opts.RemoveLineIfEmpty = true;

    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("explicit_instantiation")) {
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            auto template_kind = MSI->getTemplateSpecializationKind();
            if(template_kind == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = MSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    // std::cout << "Delete " << MFS->getNameAsString() << " at: " << loc.printToString(sm) << " (" << sm.getSpellingLineNumber(loc)
                    //           << ")" << std::endl;
                    auto begin_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1);
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1000);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
            if(TSI->getTemplateSpecializationKind() == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = TSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    // std::cout << "Delete " << MFS->getNameAsString() << " at: " << loc.printToString(Result.Context->getSourceManager()) <<
                    // std::endl;
                    auto begin_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1);
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1000);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("explicit_instantiation")) {
        if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
            if(TSI->getTemplateSpecializationKind() == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = TSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    // std::cout << "Delete " << FS->getNameAsString() << " at: " << loc.printToString(Result.Context->getSourceManager()) <<
                    // std::endl;
                    auto begin_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1);
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1000);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        }
    }
}
