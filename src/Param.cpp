#include "Param.hpp"

#include <regex>

#include "spdlog/spdlog.h"

#include "clang/AST/TemplateName.h"

Param Param::createFromParmVarDecl(const clang::ParmVarDecl* parm, const clang::PrintingPolicy pp)
{
    Param out;
    out.name = parm->getOriginalType().getAsString(pp); //.getCanonicalType()

    out.is_lvalue_reference = parm->getOriginalType().getTypePtr()->isLValueReferenceType();
    out.is_rvalue_reference = parm->getOriginalType().getTypePtr()->isRValueReferenceType();
    out.is_reference = out.is_lvalue_reference or out.is_rvalue_reference;

    spdlog::debug("Loaded parameter with name={}", out.name);
    if(out.is_reference) {
        out.is_const = parm->getOriginalType().getNonReferenceType().isConstQualified();
        out.is_volatile = parm->getOriginalType().getNonReferenceType().isVolatileQualified();
        out.is_restrict = parm->getOriginalType().getNonReferenceType().isRestrictQualified();
        out.is_dependent = parm->getOriginalType().getNonReferenceType().getTypePtr()->isDependentType();
        if(const clang::TemplateSpecializationType* TST =
               parm->getOriginalType().getCanonicalType().getNonReferenceType().getTypePtr()->getAs<const clang::TemplateSpecializationType>()) {
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
