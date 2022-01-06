#include "Template.hpp"

#include "termcolor/termcolor.hpp"

#include "Injection.hpp"
#include "Parsing.hpp"

namespace clang {
class ParmVarDecl;
}

Template Template::createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp)
{
    Template candidate;
    candidate.is_member = false;
    candidate.is_constructor = false;
    candidate.func_name = FS->getNameAsString();
    llvm::raw_string_ostream OS(candidate.nested_namespace);
    FS->printNestedNameSpecifier(OS, pp);
    OS.str();
    std::vector<clang::ParmVarDecl*> parms(FS->parameters().begin(), FS->parameters().end());
    // std::cout << "Loaded candidate (wo params): " << candidate << std::endl;
    // std::cout << "With #" << parms.size() << " parameters." << std::endl;
    candidate.params = internal::parseFunctionArgs(parms, pp);
    return candidate;
}

Template Template::createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp)
{
    Template candidate = Template::createFromFS(MFS, pp);
    candidate.is_member = true;
    const clang::CXXConstructorDecl* ConstructorCheck = llvm::dyn_cast<clang::CXXConstructorDecl>(MFS);
    candidate.is_constructor = not(ConstructorCheck == nullptr);
    candidate.is_const = MFS->isConst();
    candidate.nested_namespace = "";
    llvm::raw_string_ostream OS(candidate.nested_namespace);
    MFS->getParent()->printNestedNameSpecifier(OS, pp);
    OS.str();
    candidate.class_name = MFS->getParent()->getNameAsString();
    return candidate;
}

bool Template::isTemplateFor(const Injection& toDo) const
{
    if(is_constructor != toDo.is_constructor) { return false; }
    if(not(is_constructor) and func_name != toDo.func_name) { return false; }
    if(nested_namespace != toDo.nested_namespace) { return false; }
    if(params.size() != toDo.nonresolved_params.size()) { return false; }
    if(class_name != toDo.class_name) { return false; }
    if(is_const != toDo.is_const) { return false; }
    if(params != toDo.nonresolved_params) { return false; }
    return true;
}

std::ostream& operator<<(std::ostream& stream, const Template& candidate)
{
    stream << termcolor::bold << "•" << termcolor::reset;
    if(candidate.is_member) {
        if(candidate.is_const) {
            stream << termcolor::bold << termcolor::blue << "Const" << termcolor::reset << " member ";
        } else {
            stream << "Member ";
        }
    } else {
        stream << "Free ";
    }
    stream << "function template: " << termcolor::bold << termcolor::red << candidate.func_name << termcolor::reset;
    if(candidate.is_member) { stream << " of class " << termcolor::bold << termcolor::green << candidate.class_name << termcolor::reset; }
    stream << " of namespace: " << candidate.nested_namespace;
    stream << " with params: ";
    for(const auto& p : candidate.params) { stream << termcolor::cyan << p.name << " "; }
    stream << termcolor::reset;
    return stream;
}
