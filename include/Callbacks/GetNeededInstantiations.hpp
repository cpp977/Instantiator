#ifndef GET_NEEDED_INSTANTIATIONS_H_
#define GET_NEEDED_INSTANTIATIONS_H_

#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Injection.hpp"

class GetNeededInstantiations : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    std::vector<Injection>* toDoList;
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
