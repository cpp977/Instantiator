#include <iostream>

#include "termcolor/termcolor.hpp"

#include "Callbacks/InjectInstantiation.hpp"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "Injection.hpp"

void InjectInstantiation::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    //    pp.PrintInjectedClassNameWithArguments = 1;
    //    pp.FullyQualifiedName = 1;
    pp.PrintCanonicalTypes = 1;
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("func_definition")) {
        // std::cout << "Processing func " << MFS->getNameAsString() << std::endl;
        // std::cout << std::boolalpha << "TI=" << MFS->isTemplateInstantiation()
        //           << ", CTI=" << (MFS->getParent()->getMemberSpecializationInfo() != nullptr) << std::endl;

        llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
        if(MFS->isTemplateInstantiation() or (MFS->getParent()->getMemberSpecializationInfo() != nullptr)) {
            if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
                if(MSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }

            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present." << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(not(toDo.is_constructor) and not(toDo.func_name == MFS->getNameAsString())) {
                    it++;
                    continue;
                }
                std::string nested_namespace;
                llvm::raw_string_ostream OS(nested_namespace);
                MFS->getParent()->printNestedNameSpecifier(OS, pp);
                if(not(toDo.class_name == MFS->getParent()->getNameAsString()) or not(toDo.params.size() == params.size()) or
                   not(toDo.is_const == MFS->isConst()) or not(toDo.nested_namespace == nested_namespace)) {
                    it++;
                    continue;
                }
                bool class_tparam_match = false;
                if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
                    const clang::TemplateArgumentList& TAL = CTS->getTemplateArgs();
                    std::vector<bool> class_tparam_matches;
                    class_tparam_matches.resize(TAL.size());
                    for(std::size_t i = 0; i < TAL.size(); i++) {
                        switch(TAL.get(i).getKind()) {
                        case clang::TemplateArgument::ArgKind::Type: {
                            class_tparam_matches[i] = (toDo.class_Ttypes[i] == TAL.get(i).getAsType().getAsString(pp)); //.getCanonicalType()
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Integral: {
                            llvm::SmallString<10> name;
                            TAL.get(i).getAsIntegral().toString(name);
                            class_tparam_matches[i] = (toDo.class_Ttypes[i] == name.str().str());
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Pack: {
                            class_tparam_matches.resize(class_tparam_matches.size() + TAL.get(i).pack_size() - 1);
                            for(auto pack_it = TAL.get(i).pack_begin(); pack_it != TAL.get(i).pack_end(); pack_it++) {
                                switch(pack_it->getKind()) {
                                case clang::TemplateArgument::ArgKind::Type: {
                                    class_tparam_matches[i + std::distance(TAL.get(i).pack_begin(), pack_it)] =
                                        (toDo.class_Ttypes[i + std::distance(TAL.get(i).pack_begin(), pack_it)] ==
                                         pack_it->getAsType().getAsString(pp)); //.getCanonicalType()
                                    break;
                                }
                                case clang::TemplateArgument::ArgKind::Integral: {
                                    llvm::SmallString<10> name;
                                    pack_it->getAsIntegral().toString(name);
                                    class_tparam_matches[i + std::distance(TAL.get(i).pack_begin(), pack_it)] =
                                        (toDo.class_Ttypes[i + std::distance(TAL.get(i).pack_begin(), pack_it)] == name.str().str());
                                    break;
                                }
                                }
                            }
                            break;
                        }
                        }
                    }
                    class_tparam_match =
                        TAL.size() == 0 ? true : std::all_of(class_tparam_matches.begin(), class_tparam_matches.end(), [](bool v) { return v; });
                } else {
                    class_tparam_match = true;
                }
                bool func_tparam_match = false;
                if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                    std::vector<bool> func_tparam_matches;
                    func_tparam_matches.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        switch(TAL->get(i).getKind()) {
                        case clang::TemplateArgument::ArgKind::Type: {
                            func_tparam_matches[i] = (toDo.func_Ttypes[i] == TAL->get(i).getAsType().getAsString(pp)); //.getCanonicalType()
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Integral: {
                            llvm::SmallString<10> name;
                            TAL->get(i).getAsIntegral().toString(name);
                            func_tparam_matches[i] = (toDo.func_Ttypes[i] == name.str().str());
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Pack: {
                            func_tparam_matches.resize(func_tparam_matches.size() + TAL->get(i).pack_size() - 1);
                            for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                                switch(pack_it->getKind()) {
                                case clang::TemplateArgument::ArgKind::Type: {
                                    func_tparam_matches[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        (toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] ==
                                         pack_it->getAsType().getAsString(pp)); //.getCanonicalType()
                                    break;
                                }
                                case clang::TemplateArgument::ArgKind::Integral: {
                                    llvm::SmallString<10> name;
                                    pack_it->getAsIntegral().toString(name);
                                    func_tparam_matches[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        (toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] == name.str().str());
                                    break;
                                }
                                }
                            }
                            break;
                        }
                        }
                    }
                    func_tparam_match =
                        TAL->size() == 0 ? true : std::all_of(func_tparam_matches.begin(), func_tparam_matches.end(), [](bool v) { return v; });
                } else {
                    func_tparam_match = true;
                }
                std::vector<bool> params_matches(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    params_matches[std::distance(params.begin(), it)] =
                        ((*it)->getOriginalType().getAsString(pp) ==
                         toDo.params[std::distance(params.begin(), it)].name); // getCanonicalType().getAsString(pp)); .getCanonicalType()
                }
                bool params_match = params.size() == 0 ? true : std::all_of(params_matches.begin(), params_matches.end(), [](bool v) { return v; });
                // std::cout << std::boolalpha << "FT=" << params_match << ", CTP=" << class_tparam_match << ", FTP=" << func_tparam_match <<
                // std::endl;

                if(params_match and class_tparam_match and func_tparam_match) {
                    auto reason = Injection::createFromMFS(MFS, pp);
                    std::cout << "Erasing element from toDolist: " << std::endl
                              << toDo << std::endl
                              << "because of:" << std::endl
                              << reason.value() << std::endl;
                    it = toDoList->erase(it);
                } else {
                    // std::cout << std::boolalpha << "FT=" << params_match
                    //           << ", CTP=" << class_tparam_match
                    //           << ", FTP=" << func_tparam_match << std::endl;
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantation is needed. inject the
            // instantation in the Rewriter.
            // std::cout << "Check for match." << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                const clang::CXXConstructorDecl* ConstructorCheck = llvm::dyn_cast<clang::CXXConstructorDecl>(MFS);
                bool is_constructor = not(ConstructorCheck == nullptr);
                if(toDo.is_constructor or is_constructor) {
                    if(not(toDo.is_constructor and is_constructor)) {
                        it++;
                        continue;
                    }
                } else {
                    if(not(toDo.func_name == MFS->getNameAsString())) {
                        it++;
                        continue;
                    }
                }
                std::string nested_namespace;
                llvm::raw_string_ostream OS(nested_namespace);
                MFS->getParent()->printNestedNameSpecifier(OS, pp);
                if(not(toDo.class_name == MFS->getParent()->getNameAsString()) or not(toDo.params.size() == params.size()) or
                   not(toDo.is_const == MFS->isConst()) or not(toDo.nested_namespace == nested_namespace)) {
                    it++;
                    continue;
                }
                std::vector<bool> params_match(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    Param check = Param::createFromParmVarDecl(*it, pp);
                    // std::cout << termcolor::bold << "Compare " << check << " and " << toDo.nonresolved_params[std::distance(params.begin(), it)]
                    //           << termcolor::reset << std::endl;
                    params_match[std::distance(params.begin(), it)] = check.compare(toDo.nonresolved_params[std::distance(params.begin(), it)]);
                }
                if(std::all_of(params_match.begin(), params_match.end(), [](bool v) { return v; })) {
                    // std::cout << "Match!!! Call the rewriter and delete entry from toDoList." << std::endl;
                    rewriter->InsertText(MFS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("func_definition")) {
        // std::cout << "Processing func " << FS->getNameAsString() << std::endl;
        llvm::ArrayRef<clang::ParmVarDecl*> params = FS->parameters();
        if(FS->isTemplateInstantiation()) {
            if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
                if(TSI->getTemplateSpecializationKind() != clang::TSK_ExplicitInstantiationDefinition) { return; }
            }
            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present."
            //           << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                std::string nested_namespace;
                llvm::raw_string_ostream OS(nested_namespace);
                FS->printNestedNameSpecifier(OS, pp);
                if(not(toDo.func_name == FS->getNameAsString()) or not(toDo.params.size() == params.size()) or
                   not(toDo.nested_namespace == nested_namespace)) {
                    it++;
                    continue;
                }
                bool func_tparam_match = false;
                if(const clang::TemplateArgumentList* TAL = FS->getTemplateSpecializationArgs()) {
                    std::vector<bool> func_tparam_matches;
                    func_tparam_matches.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        switch(TAL->get(i).getKind()) {
                        case clang::TemplateArgument::ArgKind::Type: {
                            func_tparam_matches[i] = (toDo.func_Ttypes[i] == TAL->get(i).getAsType().getAsString(pp)); //.getCanonicalType()
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Integral: {
                            llvm::SmallString<10> name;
                            TAL->get(i).getAsIntegral().toString(name);
                            func_tparam_matches[i] = (toDo.func_Ttypes[i] == name.str().str());
                            break;
                        }
                        case clang::TemplateArgument::ArgKind::Pack: {
                            func_tparam_matches.resize(func_tparam_matches.size() + TAL->get(i).pack_size() - 1);
                            for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                                switch(pack_it->getKind()) {
                                case clang::TemplateArgument::ArgKind::Type: {
                                    func_tparam_matches[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        (toDo.func_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] ==
                                         pack_it->getAsType().getAsString(pp)); //.getCanonicalType()
                                    break;
                                }
                                case clang::TemplateArgument::ArgKind::Integral: {
                                    llvm::SmallString<10> name;
                                    pack_it->getAsIntegral().toString(name);
                                    func_tparam_matches[i + std::distance(TAL->get(i).pack_begin(), pack_it)] =
                                        (toDo.class_Ttypes[i + std::distance(TAL->get(i).pack_begin(), pack_it)] == name.str().str());
                                    break;
                                }
                                }
                            }
                            break;
                        }
                        }
                    }
                    func_tparam_match =
                        TAL->size() == 0 ? true : std::all_of(func_tparam_matches.begin(), func_tparam_matches.end(), [](bool v) { return v; });
                } else {
                    func_tparam_match = true;
                }
                std::vector<bool> params_matches(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    params_matches[std::distance(params.begin(), it)] =
                        ((*it)->getOriginalType().getAsString(pp) ==
                         toDo.params[std::distance(params.begin(), it)].name); // getCanonicalType().getAsString(pp)); .getCanonicalType()
                }
                bool params_match = params.size() == 0 ? true : std::all_of(params_matches.begin(), params_matches.end(), [](bool v) { return v; });
                if(params_match and func_tparam_match) {
                    // std::cout << "Erase from toDolist." << std::endl;
                    it = toDoList->erase(it);
                } else {
                    // std::cout << std::boolalpha << "FT=" << params_match
                    //           << ", FTP=" << func_tparam_match << std::endl;
                    it++;
                }
            }
        } else {
            // search in toDoList if this instantation is needed. inject the
            // instantation in the Rewriter.
            // std::cout << "Check if the correct prototype is present for
            // explicit "
            //              "instantiation."
            //           << std::endl;

            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                std::string nested_namespace;
                llvm::raw_string_ostream OS(nested_namespace);
                FS->printNestedNameSpecifier(OS, pp);
                if(not(toDo.func_name == FS->getNameAsString()) or not(toDo.params.size() == params.size()) or
                   not(toDo.nested_namespace == nested_namespace)) {
                    it++;
                    continue;
                }
                std::vector<bool> params_match(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    Param check = Param::createFromParmVarDecl(*it, pp);
                    // std::cout << termcolor::bold << "Compare " << check << " and " << toDo.nonresolved_params[std::distance(params.begin(), it)]
                    //           << termcolor::reset << std::endl;
                    params_match[std::distance(params.begin(), it)] = check.compare(toDo.nonresolved_params[std::distance(params.begin(), it)]);
                }
                if(std::all_of(params_match.begin(), params_match.end(), [](bool v) { return v; })) {
                    // std::cout
                    //     << "Match!!! Call the rewriter and delete entry from
                    //     toDoList."
                    //     << std::endl;
                    // std::cout << "Injection: " << it->getInstantiation() << std::endl;
                    rewriter->InsertText(FS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        }
    }
}
