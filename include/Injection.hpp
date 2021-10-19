#ifndef INJECTION_HPP_
#define INJECTION_HPP_

#include <string>
#include <vector>

#include "llvm/ADT/StringRef.h"

#include "clang/AST/Type.h"

struct Injection
{
    typedef std::string StringType;
    StringType func_name = "";
    StringType class_name = "";
    std::string return_type = "";
    std::vector<std::string> func_Ttypes;
    std::vector<std::string> class_Ttypes;
    std::vector<clang::QualType> params;
    bool is_const = false;
    bool is_nontemplate_member = false;
    StringType guess_injection_place;
    std::string getInstantiation() const;
};

std::ostream& operator<<(std::ostream& stream, const Injection& toDo);
#endif
