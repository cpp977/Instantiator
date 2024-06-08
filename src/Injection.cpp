#include "Injection.hpp"

#include "Parsing.hpp"
#include "termcolor/termcolor.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"

#include <ostream>
#include <sstream>

bool Injection::match(const Injection& other) const
{
    if(is_constructor != other.is_constructor) { return false; }
    if(not(is_constructor) and func_name != other.func_name) { return false; }
    if(nested_namespace != other.nested_namespace) { return false; }
    if(params.size() != other.params.size()) { return false; }
    if(class_name != other.class_name) { return false; }
    if(is_const != other.is_const) { return false; }
    if(func_Ttypes != other.func_Ttypes) { return false; }
    if(class_Ttypes != other.class_Ttypes) { return false; }
    if(params != other.params) { return false; }
    return true;
}

Injection Injection::createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp)
{
    // std::cout << "Create Injection from member function " << FS->getNameAsString() << std::endl;
    Injection toDo;
    toDo.is_member = false;
    toDo.is_constructor = false;
    toDo.func_name = FS->getNameAsString();
    toDo.return_type = FS->getReturnType().getAsString(pp);
    llvm::raw_string_ostream OS(toDo.nested_namespace);
    FS->printNestedNameSpecifier(OS, pp);
    OS.str();
    toDo.func_Ttypes = internal::parseTemplateArgs(FS->getTemplateSpecializationArgs(), pp);
    std::vector<clang::ParmVarDecl*> parms(FS->parameters().begin(), FS->parameters().end());
    toDo.params = internal::parseFunctionArgs(parms, pp);
    if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
        std::vector<clang::ParmVarDecl*> nonresolved_parms(TSI->getTemplate()->getTemplatedDecl()->parameters().begin(),
                                                           TSI->getTemplate()->getTemplatedDecl()->parameters().end());
        toDo.nonresolved_params = internal::parseFunctionArgs(nonresolved_parms, pp);
    }
    return toDo;
}

Injection Injection::createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp)
{
    // std::cout << "Create Injection from member function " << MFS->getNameAsString() << std::endl;
    Injection toDo = createFromFS(MFS, pp);
    toDo.is_member = true;
    const clang::CXXConstructorDecl* ConstructorCheck = llvm::dyn_cast<clang::CXXConstructorDecl>(MFS);
    toDo.is_constructor = not(ConstructorCheck == nullptr);
    toDo.is_const = MFS->isConst();
    toDo.nested_namespace = "";
    llvm::raw_string_ostream OS(toDo.nested_namespace);
    MFS->getParent()->printNestedNameSpecifier(OS, pp);
    OS.str();

    toDo.class_name = MFS->getParent()->getNameAsString();
    if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
        toDo.class_Ttypes = internal::parseTemplateArgs(&CTS->getTemplateArgs(), pp);
    }

    if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
        if(const clang::CXXMethodDecl* TMFS = llvm::dyn_cast<const clang::CXXMethodDecl>(MSI->getInstantiatedFrom())) {
            std::vector<clang::ParmVarDecl*> nonresolved_parms(TMFS->parameters().begin(), TMFS->parameters().end());
            toDo.nonresolved_params = internal::parseFunctionArgs(nonresolved_parms, pp);
        }
    }
    return toDo;
}

std::string Injection::getInstantiation() const
{
    std::stringstream res;
    res << "\ntemplate ";
    if(not is_constructor) { res << return_type << " "; }
    if(is_member) {
        res << class_name;
        if(class_Ttypes.size() > 0) {
            res << "<";
            for(std::size_t i = 0; i < class_Ttypes.size(); i++) {
                if(i + 1 < class_Ttypes.size()) {
                    res << class_Ttypes[i] << ",";
                } else {
                    res << class_Ttypes[i] << ">";
                }
            }
        }
        res << "::";
    }
    res << func_name;
    if(func_Ttypes.size() > 0) {
        res << "<";
        for(std::size_t i = 0; i < func_Ttypes.size(); i++) {
            if(i + 1 < func_Ttypes.size()) {
                res << func_Ttypes[i] << ",";
            } else {
                res << func_Ttypes[i] << ">";
            }
        }
    }
    res << "(";
    for(std::size_t i = 0; i < params.size(); i++) {
        if(i + 1 < params.size()) {
            res << params[i].name << ",";
        } else {
            res << params[i].name;
        }
    }
    res << ")";
    if(is_member) {
        if(is_const) { res << " const"; }
    }
    res << ";";
    return res.str();
}

std::ostream& operator<<(std::ostream& stream, const Injection& toDo)
{
    stream << termcolor::bold << "•" << termcolor::reset;
    if(toDo.is_member) {
        if(toDo.is_const) {
            stream << termcolor::bold << termcolor::blue << "Const" << termcolor::reset << " member ";
        } else {
            stream << "Member ";
        }
    } else {
        stream << "Free ";
    }
    stream << "function: " << termcolor::bold << termcolor::red << toDo.func_name << termcolor::reset;
    if(toDo.func_Ttypes.size() > 0) {
        stream << " (with template params: ";
        for(const auto& p : toDo.func_Ttypes) { stream << termcolor::magenta << p << " "; }
        stream << termcolor::reset << ")";
    }
    if(toDo.is_member) {
        stream << " of class " << termcolor::bold << termcolor::green << toDo.class_name << termcolor::reset;
        if(toDo.class_Ttypes.size() > 0) {
            stream << " (with template params: ";
            for(const auto& p : toDo.class_Ttypes) { stream << termcolor::magenta << p << " "; }
            stream << termcolor::reset << ")";
        }
    }
    stream << " of namespace: " << toDo.nested_namespace;
    stream << " with params: ";
    for(const auto& p : toDo.params) { stream << termcolor::cyan << p.name << " "; }
    stream << termcolor::reset << " with nonresolved params: ";
    for(const auto& p : toDo.nonresolved_params) { stream << termcolor::cyan << p.name << " "; }
    stream << termcolor::reset << " with return type " << termcolor::bold << termcolor::grey << toDo.return_type << termcolor::reset;
    return stream;
}
