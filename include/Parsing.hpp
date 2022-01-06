#ifndef PARSING_H_
#define PARSING_H_

#include <string>
#include <vector>

#include "clang/AST/PrettyPrinter.h"

#include "Param.hpp"

namespace clang {
class TemplateArgumentList;
class ParmVarDecl;
} // namespace clang

/** \file Parsing.hpp
 *  \brief Helper functions for parsing information from AST nodes.
 */

/**
 * Namespace for internal functions.
 */
namespace internal {
/**
 * Helper function to load template arguments.
 * \return `std::vector<std::string>` with the names of all types of template arguments.
 * \param TAL Pointer to a [clang::TemplateArgumentList](https://clang.llvm.org/doxygen/classclang_1_1TemplateArgumentList.html).
 * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
 *
 * \note Template arguments can have different kinds (types, integral values, packs, template template).
 *       The function returns the type names for typed template parameters and the values for integral parameters.
 */
std::vector<std::string> parseTemplateArgs(const clang::TemplateArgumentList* TAL, clang::PrintingPolicy pp);

/**
 * Helper function to load function parameters.
 * \return `std::vector<Param>` with an instance of Param for each parameter.
 *
 * \param Args Array of [clang::ParmVarDecl](https://clang.llvm.org/doxygen/classclang_1_1ParmVarDecl.html).
 * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
 */
std::vector<Param> parseFunctionArgs(std::vector<clang::ParmVarDecl*> Args, clang::PrintingPolicy pp);
} // namespace internal

#endif
