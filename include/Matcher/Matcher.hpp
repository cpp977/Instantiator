#ifndef MATCHERS_H_
#define MATCHERS_H_
#include "clang/ASTMatchers/ASTMatchers.h"
/**
 * \file Matcher.hpp
 * \brief [AST matcher](https://clang.llvm.org/docs/LibASTMatchersReference.html) expressions to filter the relevant nodes for template
 * instantiations.
 */

/**
 * \return Matcher expression:
 * \snippet src/Matcher/Matcher.cpp TemplInstWithoutDef
 *
 * \param excluded_names NameMatcher with namespace specifiers which should be ignored. E.g. `"std::"`
 */
clang::ast_matchers::DeclarationMatcher TemplInstWithoutDef(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names,
                                                            const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& included_names);

/**
 * \return Matcher expression:
 * \snippet src/Matcher/Matcher.cpp FuncWithDef
 *
 * \param excluded_names NameMatcher with namespace specifiers which should be ignored. E.g. `"std::"`
 *
 * \todo This matcher is not sharp enough. Only function *templates* or class *template* member functions are necessary or specializations of them.
 */
clang::ast_matchers::DeclarationMatcher FuncWithDef(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names,
                                                    const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& included_names);

/**
 * \return Matcher expression:
 * \snippet src/Matcher/Matcher.cpp TemplInst
 *
 * \param excluded_names NameMatcher with namespace specifiers which should be ignored. E.g. `"std::"`
 */
clang::ast_matchers::DeclarationMatcher TemplInst(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names,
                                                  const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& included_names);

#endif
