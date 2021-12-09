// #include <iostream>

#include "Callbacks/DeleteInstantiations.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"

#include "clang/Rewrite/Core/Rewriter.h"

void DeleteInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    pp.PrintInjectedClassNameWithArguments = true;
    pp.PrintCanonicalTypes = true;
    pp.SuppressDefaultTemplateArgs = true;
    pp.FullyQualifiedName = true;
    pp.SuppressScope = false;

    clang::SourceManager& sm = Result.Context->getSourceManager();

    clang::Rewriter::RewriteOptions opts;
    opts.IncludeInsertsAtBeginOfRange = true;
    opts.IncludeInsertsAtEndOfRange = true;
    opts.RemoveLineIfEmpty = false;

    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("explicit_instantiation")) {
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            auto template_kind = MSI->getTemplateSpecializationKind();
            if(template_kind == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = MSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    // std::cout << "Delete " << MFS->getNameAsString() << " at: " << loc.printToString(sm) << " (" << sm.getSpellingLineNumber(loc)
                    //           << ")" << std::endl;
                    auto begin_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 1);
                    // auto end_loc = clang::Lexer::getLocForEndOfToken(loc, 0, sm, Result.Context->getLangOpts());
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 2000);
                    // auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc) + 1, 1).getLocWithOffset(-1);
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
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 2000);
                    // auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc) + 1, 1).getLocWithOffset(-1);
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
                    auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc), 2000);
                    // auto end_loc = sm.translateLineCol(sm.getMainFileID(), sm.getSpellingLineNumber(loc) + 1, 1).getLocWithOffset(-1);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        }
    }
}
