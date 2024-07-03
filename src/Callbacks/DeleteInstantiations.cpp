#include "Callbacks/DeleteInstantiations.hpp"

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/TokenKinds.h>
#include <iostream>

#include "spdlog/spdlog.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"

auto findSemiAfterLocation(clang::SourceLocation loc, clang::ASTContext& Ctx, bool IsDecl = false)
{
    clang::SourceManager& SM = Ctx.getSourceManager();
    // if(loc.isMacroID()) {
    //     if(!clang::Lexer::isAtEndOfMacroExpansion(loc, SM, Ctx.getLangOpts(), &loc)) return clang::SourceLocation();
    // }
    loc = clang::Lexer::getLocForEndOfToken(loc, /*Offset=*/0, SM, Ctx.getLangOpts());

    // Break down the source location.
    std::pair<clang::FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);

    // Try to load the file buffer.
    bool invalidTemp = false;
    clang::StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
    if(invalidTemp) {
        std::cout << "invalidtemp" << std::endl;
        return std::make_pair(false, clang::SourceLocation());
    }

    const char* tokenBegin = file.data() + locInfo.second;

    // Lex from the start of the given location.
    clang::Lexer lexer(SM.getLocForStartOfFile(locInfo.first), Ctx.getLangOpts(), file.begin(), tokenBegin, file.end());
    clang::Token tok;
    lexer.LexFromRawLexer(tok);
    if(tok.isNot(clang::tok::semi)) {
        if(!IsDecl) { return std::make_pair(false, tok.getLocation()); }
        // Declaration may be followed with other tokens; such as an __attribute,
        // before ending with a semicolon.
        return findSemiAfterLocation(tok.getLocation(), Ctx, /*IsDecl*/ true);
    }

    return std::make_pair(true, tok.getLocation());
}

auto getEndLoc(clang::SourceLocation begin, clang::ASTContext& Ctx)
{
    auto end = begin;
    bool foundSemi = false;
    while(not foundSemi) {
        auto [x, y] = findSemiAfterLocation(end, Ctx);
        foundSemi = x;
        end = y;
    }
    return end;
}

auto getBeginLoc(clang::SourceLocation begin, clang::ASTContext& Ctx)
{
    clang::SourceManager& sm = Ctx.getSourceManager();
    auto line = sm.getSpellingLineNumber(begin);
    auto col = sm.getSpellingColumnNumber(begin);
    auto previous_loc = begin;
    auto previous2_loc = begin;
    auto previous3_loc = begin;
    clang::Token tok;
    while(tok.isNot(clang::tok::semi) and tok.isNot(clang::tok::r_brace)) {
        auto curr_loc = sm.translateLineCol(sm.getMainFileID(), line, col);
        auto beginCurrToken = clang::Lexer::GetBeginningOfToken(curr_loc, sm, Ctx.getLangOpts());
        line = sm.getSpellingLineNumber(beginCurrToken);
        col = sm.getSpellingColumnNumber(beginCurrToken);
        [[maybe_unused]] auto res = clang::Lexer::getRawToken(beginCurrToken, tok, sm, Ctx.getLangOpts());
        assert(col >= 1 and "Invalid column in getBeginLoc()");
        if(col > 1) {
            col = col - 1;
        } else if(col == 1) {
            line = line - 1;
            auto [fid, b_offset] = sm.getDecomposedSpellingLoc(curr_loc);
            auto offsetEndOfNextLine = b_offset - 1;
            col = sm.getColumnNumber(fid, offsetEndOfNextLine);
        }
        previous3_loc = previous2_loc;
        previous2_loc = previous_loc;
        previous_loc = beginCurrToken;
    }
    return previous3_loc;
}

void DeleteInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    pp.PrintInjectedClassNameWithArguments = true;
    pp.PrintCanonicalTypes = true;
    pp.SuppressDefaultTemplateArgs = true;
    pp.FullyQualifiedName = true;
    pp.SuppressScope = false;

    clang::Rewriter::RewriteOptions opts;
    opts.IncludeInsertsAtBeginOfRange = true;
    opts.IncludeInsertsAtEndOfRange = true;
    opts.RemoveLineIfEmpty = false;

    clang::SourceManager& sm = Result.Context->getSourceManager();

    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("explicit_instantiation")) {
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            auto template_kind = MSI->getTemplateSpecializationKind();
            if(template_kind == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = MSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    spdlog::debug("Delete {} at: {} ({})", MFS->getNameAsString(), loc.printToString(sm), sm.getSpellingLineNumber(loc));
                    auto begin_loc = getBeginLoc(loc, *Result.Context);
                    auto end_loc = getEndLoc(begin_loc, *Result.Context);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
            if(TSI->getTemplateSpecializationKind() == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = TSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    spdlog::debug("Delete {} at: {} ({})", MFS->getNameAsString(), loc.printToString(sm), sm.getSpellingLineNumber(loc));
                    auto begin_loc = getBeginLoc(loc, *Result.Context);
                    auto end_loc = getEndLoc(begin_loc, *Result.Context);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("explicit_instantiation")) {
        if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
            if(TSI->getTemplateSpecializationKind() == clang::TSK_ExplicitInstantiationDefinition) {
                auto loc = TSI->getPointOfInstantiation();
                if(loc.isValid()) {
                    spdlog::debug("Delete {} at: {} ({})", FS->getNameAsString(), loc.printToString(sm), sm.getSpellingLineNumber(loc));
                    auto begin_loc = getBeginLoc(loc, *Result.Context);
                    auto end_loc = getEndLoc(begin_loc, *Result.Context);
                    rewriter->RemoveText(clang::SourceRange(begin_loc, end_loc), opts);
                }
            }
        }
    }
}
