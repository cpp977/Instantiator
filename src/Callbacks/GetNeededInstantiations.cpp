#include "Callbacks/GetNeededInstantiations.hpp"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallString.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"

#include "Injection.hpp"

void GetNeededInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    Injection toDo;
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("templ_func_instantation")) {
        toDo.is_member = true;
        // std::cout << "Processing function " << MFS->getNameAsString() << std::endl;
        const clang::CXXConstructorDecl* ConstructorCheck = llvm::dyn_cast<clang::CXXConstructorDecl>(MFS);
        toDo.is_constructor = not(ConstructorCheck == nullptr);
        toDo.return_type = MFS->getReturnType().getCanonicalType().getAsString(pp);
        toDo.func_name = MFS->getNameAsString();
        toDo.class_name = MFS->getParent()->getNameAsString();
        if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
            const clang::TemplateArgumentList& TAL = CTS->getTemplateArgs();
            toDo.class_Ttypes.resize(TAL.size());
            for(std::size_t i = 0; i < TAL.size(); i++) {
                switch(TAL.get(i).getKind()) {
                case clang::TemplateArgument::ArgKind::Type: {
                    toDo.class_Ttypes[i] = TAL.get(i).getAsType().getCanonicalType().getAsString(pp);
                    break;
                }
                case clang::TemplateArgument::ArgKind::Integral: {
                    llvm::SmallString<10> name;
                    TAL.get(i).getAsIntegral().toString(name);
                    toDo.class_Ttypes[i] = name.str().str();
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
                            toDo.func_Ttypes[i] = TAL->get(i).getAsType().getCanonicalType().getAsString(pp);
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
                                    toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        pack_it->getAsType().getCanonicalType().getAsString(pp);
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
                        }
                    }
                }
                llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
                toDo.params.resize(params.size());
                toDo.params_name.resize(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    toDo.params[std::distance(params.begin(), it)] = (*it)->getOriginalType();
                    toDo.params_name[std::distance(params.begin(), it)] = (*it)->getOriginalType().getCanonicalType().getAsString(pp);
                }
                toDo.is_const = MFS->isConst();
                toDoList->push_back(toDo);
            }
        } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
            if(TSI->getPointOfInstantiation().isValid()) {
                if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                    toDo.func_Ttypes.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        switch(TAL->get(i).getKind()) {
                        case clang::TemplateArgument::ArgKind::Type: {
                            toDo.func_Ttypes[i] = TAL->get(i).getAsType().getCanonicalType().getAsString(pp);
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
                                    toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        pack_it->getAsType().getCanonicalType().getAsString(pp);
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
                        }
                    }
                }
                llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
                toDo.params_name.resize(params.size());
                toDo.params.resize(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    toDo.params[std::distance(params.begin(), it)] = (*it)->getOriginalType();
                    toDo.params_name[std::distance(params.begin(), it)] = (*it)->getOriginalType().getCanonicalType().getAsString(pp);
                }
                toDo.is_const = MFS->isConst();
                toDoList->push_back(toDo);
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("templ_func_instantation")) {
        toDo.is_member = false;
        toDo.is_constructor = false;
        if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
            if(TSI->getPointOfInstantiation().isValid()) {
                toDo.func_name = FS->getNameAsString();
                toDo.return_type = FS->getReturnType().getCanonicalType().getAsString(pp);
                if(const clang::TemplateArgumentList* TAL = FS->getTemplateSpecializationArgs()) {
                    toDo.func_Ttypes.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        switch(TAL->get(i).getKind()) {
                        case clang::TemplateArgument::ArgKind::Type: {
                            toDo.func_Ttypes[i] = TAL->get(i).getAsType().getCanonicalType().getAsString(pp);
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
                                    toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        pack_it->getAsType().getCanonicalType().getAsString(pp);
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
                        }
                    }
                }

                llvm::ArrayRef<clang::ParmVarDecl*> params = FS->parameters();
                toDo.params_name.resize(params.size());
                toDo.params.resize(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    toDo.params[std::distance(params.begin(), it)] = (*it)->getOriginalType();
                    toDo.params_name[std::distance(params.begin(), it)] = (*it)->getOriginalType().getCanonicalType().getAsString(pp);
                }
                toDoList->push_back(toDo);
            }
        }
    }
}
