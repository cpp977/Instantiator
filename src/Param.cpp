#include "Param.hpp"

#include "clang/AST/TemplateName.h"

#include <regex>

Param Param::createFromParmVarDecl(const clang::ParmVarDecl* parm, const clang::PrintingPolicy pp)
{
    Param out;
    // std::cout << parm->getNameAsString() << std::endl;
    // clang::QualType qt = parm->getOriginalType().getNonReferenceType();
    // const clang::Type* t = qt.getTypePtrOrNull();
    // std::cout << "nullptr: " << std::boolalpha << (t == nullptr) << std::endl;
    // // if(auto TST = t->getAs<const clang::TemplateSpecializationType>()) {
    // //     std::cout << "TST" << std::endl;
    // // } else {
    // //     std::cout << "non TST" << std::endl;
    // // }
    // if(auto* TST = llvm::dyn_cast<clang::TemplateSpecializationType>(parm->getOriginalType().getTypePtrOrNull())) {
    //     out.name = "buggy";
    //     std::cout << "Warning TST" << std::endl;
    //     clang::TemplateDecl* TD = TST->getTemplateName().getAsTemplateDecl();

    //     clang::IdentifierInfo* II = TD->getIdentifier();
    //     std::string local;
    //     llvm::raw_string_ostream OS(local);
    //     OS << II->getName();
    //     OS.str();
    //     std::cout << "local=" << local << std::endl;
    // } else {
    //     std::cout << "non TST" << std::endl;
    // }
    out.name = parm->getOriginalType().getAsString(pp); //.getCanonicalType()

    out.is_lvalue_reference = parm->getOriginalType().getTypePtr()->isLValueReferenceType();
    out.is_rvalue_reference = parm->getOriginalType().getTypePtr()->isRValueReferenceType();
    out.is_reference = out.is_lvalue_reference or out.is_rvalue_reference;

    // std::cout << termcolor::bold << "Loaded parameter with name=" << out.name << termcolor::reset << std::endl;
    if(out.is_reference) {
        out.is_const = parm->getOriginalType().getNonReferenceType().isConstQualified();
        out.is_volatile = parm->getOriginalType().getNonReferenceType().isVolatileQualified();
        out.is_restrict = parm->getOriginalType().getNonReferenceType().isRestrictQualified();
        out.is_dependent = parm->getOriginalType().getNonReferenceType().getTypePtr()->isDependentType();
        if(const clang::TemplateSpecializationType* TST =
               parm->getOriginalType().getCanonicalType().getNonReferenceType().getTypePtr()->getAs<const clang::TemplateSpecializationType>()) {
            // std::cout << "TST is non null." << std::endl;
            llvm::raw_string_ostream OS(out.template_name);
            TST->getTemplateName().print(OS, pp); // clang::TemplateName::Qualified::AsWritten
            OS.str();
        }
        out.is_template_param = parm->getOriginalType().getNonReferenceType().getTypePtr()->isTemplateTypeParmType();
    } else {
        out.is_const = parm->getOriginalType().isConstQualified();
        out.is_volatile = parm->getOriginalType().isVolatileQualified();
        out.is_restrict = parm->getOriginalType().isRestrictQualified();
        out.is_dependent = parm->getOriginalType().getTypePtr()->isDependentType();
        if(const clang::TemplateSpecializationType* TST =
               parm->getOriginalType().getCanonicalType().getTypePtr()->getAs<const clang::TemplateSpecializationType>()) {
            llvm::raw_string_ostream OS(out.template_name);
            TST->getTemplateName().print(OS, pp); // clang::TemplateName::Qualified::AsWritten
            OS.str();
        }
        out.is_template_param = parm->getOriginalType().getTypePtr()->isTemplateTypeParmType();
    }
    if(out.is_rvalue_reference and out.is_template_param) { out.is_forwarding_reference = true; }
    // out.dependence = parm->getOriginalType().getTypePtr()->getDependence();
    return out;
}

bool Param::compare_cvr(const Param& other) const
{
    return is_const == other.is_const and is_volatile == other.is_volatile and is_restrict == other.is_restrict;
}

bool Param::compare(const Param& other) const
{
    std::regex r("type-parameter-[0-9]+-[0-9]+");
    auto name_corrected = std::regex_replace(name, r, "type-parameter-X-Y");
    auto other_name_corrected = std::regex_replace(other.name, r, "type-parameter-X-Y");
    return name_corrected == other_name_corrected;
}
