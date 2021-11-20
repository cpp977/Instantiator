#ifndef INJECTION_HPP_
#define INJECTION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "llvm/ADT/StringRef.h"

#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"

#include "Param.hpp"

/**
 * \brief Struct for the collection of all relevant data for a template instantiation which needs to be inserted somewhere.
 *
 */
struct Injection
{
    typedef std::string StringType;
    /**
     * Name of the function without namespace and template parameters.
     * For \code{.cpp}void Bar::Foo<T1,T2>(const T1& t1, const T2& t2)\endcode the name would be `"Foo"`.
     */
    StringType func_name = "";
    /**
     * Nested namespace of the function. For
     * For \code{.cpp}void My::Nested::Namespace::Foo<T1,T2>(const T1& t1, const T2& t2)\endcode the nested namespace would be
     * `"My::Nested::Namespace"`.
     */
    StringType nested_namespace = "";
    /**
     * For member functions, this is the name of the parent class. For free functions it is empty.
     * E.g. \code{.cpp}void Bar::Foo<T1,T2>(const T1& t1, const T2& t2)\endcode the class name would be `"Bar"`.
     */
    StringType class_name = "";
    /**
     * The return type of the function.
     */
    std::string return_type = "";
    /**
     * The concrete types of the function template parameters.
     * E.g. \code{.cpp}void Bar::Foo<double,int>(const double& t1, const int& t2)\endcode it would be `{"double", "int"}`
     */
    std::vector<std::string> func_Ttypes;
    /**
     * The concrete types of the class template parameters (for class template member functions only).
     * E.g. \code{.cpp}void Bar<double, int>::Foo()\endcode it would be `{"double", "int"}`
     */
    std::vector<std::string> class_Ttypes;
    /**
     * A vector of the function parameters where each parameter is fully resolved with the concrete types.
     * This information is needed to create the explicit instantiation.
     */
    std::vector<Param> params;
    /**
     * A vector of the function parameters where the parameters are not resolved with concrete types.
     * If all of the parameter are nondependent on template parameters its the same as params.
     * This information is needed to compare this Injection with possible candidates which provide the appropriate definition.
     */
    std::vector<Param> nonresolved_params;
    // std::vector<std::string> params_name;
    bool is_const = false;
    bool is_member = false;
    bool is_constructor = false;
    StringType guess_injection_place;
    std::string getInstantiation() const;
    static std::optional<Injection> createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp);
    static std::optional<Injection> createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp);
};

std::ostream& operator<<(std::ostream& stream, const Injection& toDo);
#endif
