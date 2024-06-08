#ifndef TEMPLATE_HPP_
#define TEMPLATE_HPP_
#include "Injection.hpp"
#include "Param.hpp"

#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"

#include <string>
#include <vector>

/**
 * \brief Struct for the collection of all relevant data for a template function which can provide a definition for an explicit instantiation from a
 * different translation unit.
 */
struct Template
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
     * A vector of the function parameters.
     *
     * \note
     * The parameters are nonresolved because this is the signature of a function template.
     */
    std::vector<Param> params;

    /**Whether this member function is const qualified.*/
    bool is_const = false;

    /**Whether this is a member function of some class.*/
    bool is_member = false;

    /**
     * Whether this a class constructor.
     * \note Names of class constructors are special in c++. We therefore don't need to compare it when we search for matches of constructors.
     */
    bool is_constructor = false;

    /**
     * Checks whether `this`  is a matching template for the Injection \p candidate. I.e. the following is checked:
     *  - function name
     *  - nonresolved function parameters of \p candidate with function parameters of the template
     *  - namespace
     *  - const qualifier
     *  - constructor bit
     */
    bool isTemplateFor(const Injection& candidate) const;

    /**
     * Function to load all needed data from a [clang::CXXMethodDecl](https://clang.llvm.org/doxygen/classclang_1_1CXXMethodDecl.html).
     * \return Data bundled into a Template instance.
     *
     * \param MFS Pointer to a `clang::CXXMethodDecl`.
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     *
     * Calls createFromFS() and loads the following member function information:
     *  - Sets is_member to true.
     *  - Determine whether it is a contructor -> is_constructor
     *  - Determines whether this is a const qualified member function -> is_const
     *  - Resets the nested namespace qualifier to the nested namespace of the parent class -> nested_namespace
     *  - Class name of parent class -> class_name
     */
    static Template createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp);

    /**
     * Function to load all needed data from a [clang::FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html).
     * \return Data bundled into a Template instance.
     *
     * \param FS Pointer to a `clang::FunctionDecl`.
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     *
     * The following data is parsed:
     *  - Function name -> func_name
     *  - Nested namespace in which the function is defined -> nested_namespace
     *  - Function parameters of the template -> params
     */
    static Template createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp);
};

/**Pretty print a \p Template*/
std::ostream& operator<<(std::ostream& stream, const Template& toDo);
#endif
