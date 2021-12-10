#include "Injection.hpp"

#include <ostream>
#include <sstream>

#include "termcolor/termcolor.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"

std::optional<Injection> Injection::createFromFS(const clang::FunctionDecl* FS, clang::PrintingPolicy pp)
{
    // std::cout << "Create Injection from member function " << FS->getNameAsString() << std::endl;
    Injection toDo;
    toDo.is_member = false;
    toDo.is_constructor = false;
    if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
        if(TSI->getPointOfInstantiation().isValid()) {
            toDo.func_name = FS->getNameAsString();
            llvm::raw_string_ostream OS(toDo.nested_namespace);
            FS->printNestedNameSpecifier(OS, pp);
            OS.str();
            toDo.return_type = FS->getReturnType().getAsString(pp);
            if(const clang::TemplateArgumentList* TAL = FS->getTemplateSpecializationArgs()) {
                toDo.func_Ttypes.resize(TAL->size());
                for(std::size_t i = 0; i < TAL->size(); i++) {
                    switch(TAL->get(i).getKind()) {
                    case clang::TemplateArgument::ArgKind::Type: {
                        toDo.func_Ttypes[i] = TAL->get(i).getAsType().getAsString(pp);
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Integral: {
                        llvm::SmallString<10> name;
                        TAL->get(i).getAsIntegral().toString(name);
                        toDo.func_Ttypes[i] = name.str().str();
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Pack: {
                        toDo.func_Ttypes.resize(toDo.func_Ttypes.size() + TAL->get(i).pack_size() - 1);
                        for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                            switch(pack_it->getKind()) {
                            case clang::TemplateArgument::ArgKind::Type: {
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = pack_it->getAsType().getAsString(pp);
                                break;
                            }
                            case clang::TemplateArgument::ArgKind::Integral: {
                                llvm::SmallString<10> name;
                                pack_it->getAsIntegral().toString(name);
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = name.str().str();
                                break;
                            }
                            }
                        }
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Template: {
                        llvm::raw_string_ostream OS(toDo.func_Ttypes[i]);
#if INSTANTIATOR_LLVM_MAJOR > 13
                        TAL->get(i).getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::Fully);
#else
                        TAL->get(i).getAsTemplate().print(OS, pp, false);
#endif
                        OS.str();
                        break;
                    }
                    }
                }
            }

            llvm::ArrayRef<clang::ParmVarDecl*> params = FS->parameters();
            toDo.params.resize(params.size());
            for(auto it = params.begin(); it != params.end(); it++) {
                toDo.params[std::distance(params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
            }
            llvm::ArrayRef<clang::ParmVarDecl*> nonresolved_params = TSI->getTemplate()->getTemplatedDecl()->parameters();
            toDo.nonresolved_params.resize(nonresolved_params.size());
            for(auto it = nonresolved_params.begin(); it != nonresolved_params.end(); it++) {
                toDo.nonresolved_params[std::distance(nonresolved_params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
            }
            std::optional<Injection> out;
            out = toDo;
            // std::cout << termcolor::green << "Created with success." << termcolor::reset << std::endl;
            return out;
        }
    }
    std::optional<Injection> out;
    // std::cout << termcolor::red << "Not created." << termcolor::reset << std::endl;
    return out;
}

std::optional<Injection> Injection::createFromMFS(const clang::CXXMethodDecl* MFS, clang::PrintingPolicy pp)
{
    // std::cout << "Create Injection from member function " << MFS->getNameAsString() << std::endl;
    Injection toDo;
    toDo.is_member = true;
    const clang::CXXConstructorDecl* ConstructorCheck = llvm::dyn_cast<clang::CXXConstructorDecl>(MFS);
    toDo.is_constructor = not(ConstructorCheck == nullptr);
    toDo.return_type = MFS->getReturnType().getAsString(pp);
    toDo.func_name = MFS->getNameAsString();
    llvm::raw_string_ostream OS(toDo.nested_namespace);
    MFS->getParent()->printNestedNameSpecifier(OS, pp);
    OS.str();
    toDo.class_name = MFS->getParent()->getNameAsString();
    if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
        const clang::TemplateArgumentList& TAL = CTS->getTemplateArgs();
        toDo.class_Ttypes.resize(TAL.size());
        for(std::size_t i = 0; i < TAL.size(); i++) {
            switch(TAL.get(i).getKind()) {
            case clang::TemplateArgument::ArgKind::Type: {
                toDo.class_Ttypes[i] = TAL.get(i).getAsType().getAsString(pp);
                break;
            }
            case clang::TemplateArgument::ArgKind::Integral: {
                llvm::SmallString<10> name;
                TAL.get(i).getAsIntegral().toString(name);
                toDo.class_Ttypes[i] = name.str().str();
                break;
            }
            case clang::TemplateArgument::ArgKind::Template: {
                llvm::raw_string_ostream OS(toDo.class_Ttypes[i]);
#if INSTANTIATOR_LLVM_MAJOR > 13
                TAL.get(i).getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::Fully);
#else
                TAL.get(i).getAsTemplate().print(OS, pp, false);
#endif
                OS.str();
                break;
            }
            }
        }
    }
    if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
        if(MSI->getPointOfInstantiation().isValid()) {
            if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                toDo.func_Ttypes.resize(TAL->size());
                for(std::size_t i = 0; i < TAL->size(); i++) {
                    switch(TAL->get(i).getKind()) {
                    case clang::TemplateArgument::ArgKind::Type: {
                        toDo.func_Ttypes[i] = TAL->get(i).getAsType().getAsString(pp);
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Integral: {
                        llvm::SmallString<10> name;
                        TAL->get(i).getAsIntegral().toString(name);
                        toDo.func_Ttypes[i] = name.str().str();
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Pack: {
                        toDo.func_Ttypes.resize(toDo.func_Ttypes.size() + TAL->get(i).pack_size() - 1);
                        for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                            switch(pack_it->getKind()) {
                            case clang::TemplateArgument::ArgKind::Type: {
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = pack_it->getAsType().getAsString(pp);
                                break;
                            }
                            case clang::TemplateArgument::ArgKind::Integral: {
                                llvm::SmallString<10> name;
                                pack_it->getAsIntegral().toString(name);
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = name.str().str();
                                break;
                            }
                            }
                        }
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Template: {
                        llvm::raw_string_ostream OS(toDo.func_Ttypes[i]);
#if INSTANTIATOR_LLVM_MAJOR > 13
                        TAL->get(i).getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::Fully);
#else
                        TAL->get(i).getAsTemplate().print(OS, pp, false);
#endif
                        OS.str();
                        break;
                    }
                    }
                }
            }
            llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
            toDo.params.resize(params.size());
            for(auto it = params.begin(); it != params.end(); it++) {
                toDo.params[std::distance(params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
            }
            if(const clang::CXXMethodDecl* TMFS = llvm::dyn_cast<const clang::CXXMethodDecl>(MSI->getInstantiatedFrom())) {
                llvm::ArrayRef<clang::ParmVarDecl*> nonresolved_params = TMFS->parameters();
                toDo.nonresolved_params.resize(nonresolved_params.size());
                for(auto it = nonresolved_params.begin(); it != nonresolved_params.end(); it++) {
                    toDo.nonresolved_params[std::distance(nonresolved_params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
                }
            }

            toDo.is_const = MFS->isConst();
            std::optional<Injection> out;
            out = toDo;
            // std::cout << termcolor::green << "Created with success." << termcolor::reset << std::endl;
            return out;
        }
    } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
        if(TSI->getPointOfInstantiation().isValid()) {
            if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                toDo.func_Ttypes.resize(TAL->size());
                for(std::size_t i = 0; i < TAL->size(); i++) {
                    switch(TAL->get(i).getKind()) {
                    case clang::TemplateArgument::ArgKind::Type: {
                        toDo.func_Ttypes[i] = TAL->get(i).getAsType().getAsString(pp);
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Integral: {
                        llvm::SmallString<10> name;
                        TAL->get(i).getAsIntegral().toString(name);
                        toDo.func_Ttypes[i] = name.str().str();
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Pack: {
                        toDo.func_Ttypes.resize(toDo.func_Ttypes.size() + TAL->get(i).pack_size() - 1);
                        for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                            switch(pack_it->getKind()) {
                            case clang::TemplateArgument::ArgKind::Type: {
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = pack_it->getAsType().getAsString(pp);
                                break;
                            }
                            case clang::TemplateArgument::ArgKind::Integral: {
                                llvm::SmallString<10> name;
                                pack_it->getAsIntegral().toString(name);
                                toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = name.str().str();
                                break;
                            }
                            }
                        }
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Template: {
                        llvm::raw_string_ostream OS(toDo.func_Ttypes[i]);
#if INSTANTIATOR_LLVM_MAJOR > 13
                        TAL->get(i).getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::Fully);
#else
                        TAL->get(i).getAsTemplate().print(OS, pp, false);
#endif
                        OS.str();
                        break;
                    }
                    }
                }
            }
            llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
            toDo.params.resize(params.size());
            for(auto it = params.begin(); it != params.end(); it++) {
                toDo.params[std::distance(params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
            }

            llvm::ArrayRef<clang::ParmVarDecl*> nonresolved_params = TSI->getTemplate()->getTemplatedDecl()->parameters();
            toDo.nonresolved_params.resize(nonresolved_params.size());
            for(auto it = nonresolved_params.begin(); it != nonresolved_params.end(); it++) {
                toDo.nonresolved_params[std::distance(nonresolved_params.begin(), it)] = Param::createFromParmVarDecl(*it, pp);
            }
            toDo.is_const = MFS->isConst();
            std::optional<Injection> out;
            out = toDo;
            // std::cout << termcolor::green << "Created with success." << termcolor::reset << std::endl;
            return out;
        }
    }
    std::optional<Injection> out;
    // std::cout << termcolor::red << "Not created." << termcolor::reset << std::endl;
    return out;
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
