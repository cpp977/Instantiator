#ifndef MATCHERS_H_
#define MATCHERS_H_
#include "clang/ASTMatchers/ASTMatchers.h"

clang::ast_matchers::DeclarationMatcher TemplateInstantiations(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names);

clang::ast_matchers::DeclarationMatcher Candidates(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names);

clang::ast_matchers::DeclarationMatcher ExplicitInstantiations(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names);
#endif
