#ifndef PARAM_H_
#define PARAM_H_

#include "clang/AST/Decl.h"
#include "clang/AST/DependenceFlags.h"
#include "clang/AST/PrettyPrinter.h"

#include <ostream>
#include <string>

/**
 * \brief Struct for the collection of all relevant data for a function parameter.
 */
struct Param
{
    /**Name of the function*/
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

    /**
     * Load the parameter from a [clang::ParmVarDecl](https://clang.llvm.org/doxygen/classclang_1_1ParmVarDecl.html).
     * \param parm Pointer to the `clang::ParmVarDecl`
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     */
    static Param createFromParmVarDecl(const clang::ParmVarDecl* parm, clang::PrintingPolicy pp);

    /**
     * Compares the `const`, `volatile` and `restrict` qualifier of the parameter type.
     */
    bool compare_cvr(const Param& other) const;

    /**
     * Compares if two parameters are the same.
     */
    bool compare(const Param& other) const;

    inline bool operator==(const Param& other) const { return this->compare(other); }
};

/**Pretty print a \p Param*/
std::ostream& operator<<(std::ostream& stream, const Param& p);
#endif
