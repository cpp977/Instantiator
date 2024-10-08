#include "Injection.hpp"

#include <ostream>
#include <sstream>

#include "spdlog/spdlog.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"

#include "Parsing.hpp"

bool Injection::match(const Injection& other) const
{
    if(is_constructor != other.is_constructor) { return false; }
    if(not(is_constructor) and func_name != other.func_name) { return false; }
    if(nested_namespace != other.nested_namespace) { return false; }
    if(params.size() != other.params.size()) { return false; }
    if(class_name != other.class_name) { return false; }
    if(is_const != other.is_const) { return false; }
    if(func_Ttypes != other.func_Ttypes) { return false; }
    if(class_Targs != other.class_Targs) { return false; }
    if(params != other.params) { return false; }

    return true;
}

Injection Injection::createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp)
{
    spdlog::debug("Create Injection from function {}.", FS->getNameAsString());
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
    spdlog::debug("Create Injection from member function {}.", MFS->getNameAsString());
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
        auto targs = &CTS->getTemplateArgs();
        toDo.class_Targs.reserve(targs->size());
        for(std::size_t i = 0; i < targs->size(); ++i) {
            toDo.class_Targs.push_back(Instantiator::TemplateArgument::createFromTemplateArgument(&targs->get(i), pp));
        }
    }

    if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
        auto* node = TSI->getTemplate()->getTemplatedDecl();
        if(auto* DFT = node->getDescribedFunctionTemplate()) {
            if(auto* MT = DFT->getInstantiatedFromMemberTemplate()) {
                if(auto* TMFS = MT->getTemplatedDecl()) {
                    std::vector<clang::ParmVarDecl*> nonresolved_parms(TMFS->parameters().begin(), TMFS->parameters().end());
                    toDo.nonresolved_params = internal::parseFunctionArgs(nonresolved_parms, pp);
                }
            }
        }
    } else {
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            if(const clang::CXXMethodDecl* TMFS = llvm::dyn_cast<const clang::CXXMethodDecl>(MSI->getInstantiatedFrom())) {
                std::vector<clang::ParmVarDecl*> nonresolved_parms(TMFS->parameters().begin(), TMFS->parameters().end());
                toDo.nonresolved_params = internal::parseFunctionArgs(nonresolved_parms, pp);
            }
        }
    }
    return toDo;
}

std::string Injection::getInstantiation() const
{
    std::stringstream res;
    res << "\ntemplate ";
    if(not is_constructor) { res << return_type << " "; }
    res << nested_namespace;
    if(is_member) {
        res << class_name;
        if(class_Targs.size() > 0) {
            res << "<";
            for(std::size_t i = 0; i < class_Targs.size(); i++) {
                if(i + 1 < class_Targs.size()) {
                    res << class_Targs[i].name(",") << ",";
                } else {
                    res << class_Targs[i].name(",") << ">";
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
