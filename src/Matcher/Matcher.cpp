#include "Matcher/Matcher.hpp"

clang::ast_matchers::DeclarationMatcher TemplateInstantiations(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names)
{
    return clang::ast_matchers::functionDecl(clang::ast_matchers::isTemplateInstantiation(),
                                             clang::ast_matchers::unless(clang::ast_matchers::isDefinition()),
                                             clang::ast_matchers::unless(excluded_names))
        .bind("templ_func_instantation");
}

clang::ast_matchers::DeclarationMatcher Candidates(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names)
{
    return clang::ast_matchers::functionDecl(clang::ast_matchers::isDefinition(), clang::ast_matchers::unless(excluded_names))
        .bind("func_definition");
}

clang::ast_matchers::DeclarationMatcher ExplicitInstantiations(const clang::ast_matchers::internal::Matcher<clang::NamedDecl>& excluded_names)
{
    return clang::ast_matchers::functionDecl(clang::ast_matchers::isTemplateInstantiation(), clang::ast_matchers::unless(excluded_names))
        .bind("explicit_instantiation");
}
