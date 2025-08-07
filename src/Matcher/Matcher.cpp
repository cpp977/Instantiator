#include "Matcher/Matcher.hpp"

using namespace clang::ast_matchers;

DeclarationMatcher TemplInstWithoutDef(const internal::Matcher<clang::NamedDecl>& excluded_names, const internal::Matcher<clang::NamedDecl>& included_names)
{
    //! [TemplInstWithoutDef]
    return functionDecl(isTemplateInstantiation(), unless(isDefinition()), unless(excluded_names), included_names).bind("templ_func_instantation");
    //! [TemplInstWithoutDef]
}

DeclarationMatcher FuncWithDef(const internal::Matcher<clang::NamedDecl>& excluded_names, const internal::Matcher<clang::NamedDecl>& included_names)
{
    //! [FuncWithDef]
    return functionDecl(isDefinition(), unless(excluded_names), included_names).bind("func_definition");
    //! [FuncWithDef]
}

DeclarationMatcher TemplInst(const internal::Matcher<clang::NamedDecl>& excluded_names, const internal::Matcher<clang::NamedDecl>& included_names)
{
    //! [TemplInst]
    return functionDecl(isTemplateInstantiation(), unless(excluded_names), included_names).bind("explicit_instantiation");
    //! [TemplInst]
}
