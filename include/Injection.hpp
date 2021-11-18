#ifndef INJECTION_HPP_
#define INJECTION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "llvm/ADT/StringRef.h"

#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"

#include "Param.hpp"

struct Injection
{
    typedef std::string StringType;
    StringType func_name = "";
    StringType nested_namespace = "";
    StringType class_name = "";
    std::string return_type = "";
    std::vector<std::string> func_Ttypes;
    std::vector<std::string> class_Ttypes;
    std::vector<Param> params;
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
