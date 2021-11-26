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

    /**Whether this member function is const qualified.*/
    bool is_const = false;

    /**Whether this is a member function of some class.*/
    bool is_member = false;

    /**
     * Whether this a class constructor.
     * \note Names of class constructors are special in c++. We therefore don't need to compare it when we search for matches of constructors.
     */
    bool is_constructor = false;

    /**Unused so far. Intended for more efficiency in the lookup step.*/
    StringType guess_injection_place;

    /**
     * \return The string which needs to inserted to explicitly instantiate this function template or class template member function.
     */
    std::string getInstantiation() const;

    /**
     * Function to load all needed data from a [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html).
     * \return If \p MFS is appropriate the Injection otherwise an empty `std::optional`.
     *
     * \param MFS Pointer to a `clang::CXXMethodDecl`.
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     * \todo Remove duplicate code by using code from `createFromFS()`.
     */
    static std::optional<Injection> createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp);

    /**
     * Function to load all needed data from a [clang::FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html).
     * \return If \p FS is appropriate the Injection otherwise an empty `std::optional`.
     *
     * \param FS Pointer to a `clang::FunctionDecl`.
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     */
    static std::optional<Injection> createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp);
};

/**Pretty print an \p Injection*/
std::ostream& operator<<(std::ostream& stream, const Injection& toDo);
#endif