#ifndef INJECT_INSTANTIATION_H_
#define INJECT_INSTANTIATION_H_

#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Injection.hpp"

// forward declaration
namespace clang {
class Rewriter;
}

class InjectInstantiation : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    std::vector<Injection>* toDoList;
    clang::Rewriter* rewriter;

    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
