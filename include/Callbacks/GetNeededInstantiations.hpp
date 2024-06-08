#ifndef GET_NEEDED_INSTANTIATIONS_H_
#define GET_NEEDED_INSTANTIATIONS_H_

#include "Injection.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <vector>

/**
 * \brief MatchCallback for template instantiation detections.
 *
 * This callback is called for the Matcher TemplInstWithoutDef().
 * The Matcher returns a [clang::FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html) or a
 * [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html) which has **no** definition in this translation unit
 * and which is a template instantiation. This callback checks whether these instantiations are valid and then load the relevant data as an
 * Injection.
 */
class GetNeededInstantiations : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    /**
     * A pointer to an external vector of Injection which is filled during GetNeededInstantiations.
     * \todo Can this be a `std::list`?
     */
    std::vector<Injection>* toDoList;
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
