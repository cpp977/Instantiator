#ifndef DELETE_INSTANTIATIONS_H_
#define DELETE_INSTANTIATIONS_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"

class DeleteInstantiations : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    clang::Rewriter* rewriter;
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
