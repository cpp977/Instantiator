#ifndef INJECT_INSTANTIATION_H_
#define INJECT_INSTANTIATION_H_

#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Injection.hpp"

// forward declaration
namespace clang {
class Rewriter;
}

/**
 * \brief MatchCallback for injection of explicit instantiations into the source files at appropriate places.
 *
 * This callback is called for the Matcher FuncWithDef().
 * The Matcher returns a [clang::FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html) or a
 * [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html) which has a definition in this translation unit.
 * This callback checks whether there is an Injection item in the toDoList which which fits the current match result.
 * The following is compared for each match result:
 * -# Return type
 * -# Nested namespace qualifier
 * -# Function name
 * -# For [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html) `const` qualifier
 * -# For [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html) parent class name
 * -# Function parameters
 */
class InjectInstantiation : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    /**
     * A pointer to an external vector of Injection which is filled during GetNeededInstantiations.
     */
    std::vector<Injection>* toDoList;
    /**Pointer to an external [clang::Rewriter]((https://clang.llvm.org/doxygen/classclang_1_1Rewriter.html)) object.*/
    clang::Rewriter* rewriter;

    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
