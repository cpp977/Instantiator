#include "Param.hpp"

#include <regex>

#include "termcolor/termcolor.hpp"

#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"

Param Param::createFromParmVarDecl(const clang::ParmVarDecl* parm, const clang::PrintingPolicy pp)
{
    Param out;
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
    // if(auto it1 = name.find("type-parameter"), it2 = other.name.find("type-parameter"); it1 != std::string::npos and it2 != std::string::npos) {
    //     return true;
    // }
    std::regex r("type-parameter-[0-9]+-[0-9]+");
    auto name_corrected = std::regex_replace(name, r, "type-parameter-X-Y");
    auto other_name_corrected = std::regex_replace(other.name, r, "type-parameter-X-Y");
    return name_corrected == other_name_corrected;
    if(is_template_param or other.is_template_param) {
        if(is_forwarding_reference or other.is_forwarding_reference) { return true; }
        return compare_cvr(other);
    } else {
        if(template_name != "" and other.template_name != "") { return compare_cvr(other) and template_name == other.template_name; }
        return compare_cvr(other) and name == other.name;
    }
}

std::ostream& operator<<(std::ostream& stream, const Param& p)
{
    stream << termcolor::bold << "•" << termcolor::reset;
    stream << ((p.is_const) ? termcolor::green : termcolor::red) << "const " << termcolor::reset;
    stream << ((p.is_volatile) ? termcolor::green : termcolor::red) << "volatile " << termcolor::reset;
    stream << ((p.is_restrict) ? termcolor::green : termcolor::red) << "restrict " << termcolor::reset;
    stream << termcolor::bold << p.name << termcolor::reset;
    if(p.is_dependent) { stream << ", dependent"; }
    if(p.is_template_param) { stream << termcolor::on_red << ", template" << termcolor::reset; }
    stream << ", template_name=" << termcolor::magenta << p.template_name << termcolor::reset;
    return stream;
}
