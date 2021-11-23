#ifndef DELETE_INSTANTIATIONS_H_
#define DELETE_INSTANTIATIONS_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"

// forward declaration
namespace clang {
class Rewriter;
}

/**
 * \brief MatchCallback for template instantiation deletions.
 *
 * This callback is called for the Matcher TemplInst().
 * It detects the point of instantiation in the source code and registers the deletion in the
 * [clang::Rewriter](https://clang.llvm.org/doxygen/classclang_1_1Rewriter.html). For example in this piece of code:
 * \code{.cpp}
 * template<typename Foo>
 * void doSomething(Foo& foo) {
 *     //some code
 * }
 * template void doSomething(double&);
 * \endcode
 * the callback determines the line `template void doSomething(double&);`.
 * \todo
 * The callback uses
 * [clang::FunctionTemplateSpecializationInfo::getPointOfInstantiation()](https://clang.llvm.org/doxygen/classclang_1_1FunctionTemplateSpecializationInfo.html#a8455a2551d86b9bbdeb904858dcb458d)
 * which gives the [clang::SourceLocation](https://clang.llvm.org/doxygen/classclang_1_1SourceLocation.html) of the beginning of the explicit
 * instantiation. In the example above that would be the column corresponding to the beginning of `doSomething`. How can one get the begin of the
 * whole declaration and how can one get the end of the declaration?
 */
class DeleteInstantiations : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    /**Pointer to an external [clang::Rewriter]((https://clang.llvm.org/doxygen/classclang_1_1Rewriter.html)) object.*/
    clang::Rewriter* rewriter;
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
};

#endif
