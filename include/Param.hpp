#ifndef PARAM_H_
#define PARAM_H_

#include <ostream>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/DependenceFlags.h"
#include "clang/AST/PrettyPrinter.h"

struct Param
{
    std::string name = "";
    bool is_lvalue_reference = false;
    bool is_rvalue_reference = false;
    bool is_forwarding_reference = false;
    bool is_reference = false;
    bool is_const = false;
    bool is_volatile = false;
    bool is_restrict = false;
    bool is_dependent = false;
    std::string template_name = "";
    bool is_template_param = false;
    // bool is_templateparmvartype = false;
    // clang::TypeDependenceScope::TypeDependence dependence = clang::TypeDependenceScope::TypeDependence::None;
    static Param createFromParmVarDecl(const clang::ParmVarDecl* parm, clang::PrintingPolicy pp, bool PrintCanonicalTypes = true);
    bool compare_cvr(const Param& other) const;
    bool compare(const Param& other) const;
};

std::ostream& operator<<(std::ostream& stream, const Param& p);
#endif
