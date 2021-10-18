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
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("func_definition")) {
        // std::cout << "Processing func " << MFS->getNameAsString() << std::endl;
        llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
        if(MFS->isTemplateInstantiation()) {
            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present."
            //           << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(not(toDo.func_name == MFS->getName()) or not(toDo.class_name == MFS->getParent()->getName()) or
                   not(toDo.params.size() == params.size()) or not(toDo.is_const == MFS->isConst())) {
                    it++;
                    continue;
                }
                bool class_tparam_match = false;
                if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
                    const clang::TemplateArgumentList& TAL = CTS->getTemplateArgs();
                    std::vector<bool> class_tparam_matches;
                    class_tparam_matches.resize(TAL.size());
                    for(std::size_t i = 0; i < TAL.size(); i++) {
                        class_tparam_matches[i] = (toDo.class_Ttypes[i] == TAL.get(i).getAsType().getCanonicalType().getAsString());
                    }
                    class_tparam_match =
                        TAL.size() == 0 ? true : std::all_of(class_tparam_matches.begin(), class_tparam_matches.end(), [](bool v) { return v; });
                }
                bool func_tparam_match = false;
                if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                    std::vector<bool> func_tparam_matches;
                    func_tparam_matches.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        func_tparam_matches[i] = (toDo.func_Ttypes[i] == TAL->get(i).getAsType().getCanonicalType().getAsString());
                    }
                    func_tparam_match =
                        TAL->size() == 0 ? true : std::all_of(func_tparam_matches.begin(), func_tparam_matches.end(), [](bool v) { return v; });
                } else {
                    func_tparam_match = true;
                }
                std::vector<bool> params_matches(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    params_matches[std::distance(params.begin(), it)] =
                        ((*it)->getOriginalType().getCanonicalType().getAsString() ==
                         toDo.params[std::distance(params.begin(), it)].getCanonicalType().getAsString());
                }
                bool params_match = params.size() == 0 ? true : std::all_of(params_matches.begin(), params_matches.end(), [](bool v) { return v; });
                if(params_match and class_tparam_match and func_tparam_match) {
                    // std::cout << "Erase from toDolist." << std::endl;
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
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(not(toDo.func_name == MFS->getName()) or not(toDo.class_name == MFS->getParent()->getName()) or
                   not(toDo.params.size() == params.size()) or not(toDo.is_const == MFS->isConst())) {
                    it++;
                    continue;
                }
                std::vector<bool> params_match(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    if(not(*(*it)->getOriginalType()).isDependentType()) { /*Parameter is not a dependent type
                                                                              -> check for exact matching.*/
                        params_match[std::distance(params.begin(), it)] =
                            ((*it)->getOriginalType().getCanonicalType().getAsString() ==
                             toDo.params[std::distance(params.begin(), it)].getCanonicalType().getAsString());
                    } else { /*Parameter is a dependent type -> check for cvr and
                                reference matching.*/
                        params_match[std::distance(params.begin(), it)] =
                            (((*it)->getOriginalType().getTypePtr()->isReferenceType() ==
                              toDo.params[std::distance(params.begin(), it)].getTypePtr()->isReferenceType()) and
                             ((*it)->getOriginalType().getNonReferenceType().getQualifiers() ==
                              toDo.params[std::distance(params.begin(), it)].getNonReferenceType().getQualifiers()));
                    }
                }
                if(std::all_of(params_match.begin(), params_match.end(), [](bool v) { return v; })) {
                    // std::cout
                    //     << "Match!!! Call the rewriter and delete entry from
                    //     toDoList."
                    //     << std::endl;
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
            // search in toDoList if this instantation is needed. if yes -> delete
            // it from list.
            // std::cout << "Check if the instantiation is already present."
            //           << std::endl;
            for(auto it = toDoList->begin(); it != toDoList->end();) {
                Injection& toDo = *it;
                if(not(toDo.func_name == FS->getName()) or not(toDo.params.size() == params.size())) {
                    it++;
                    continue;
                }
                bool func_tparam_match = false;
                if(const clang::TemplateArgumentList* TAL = FS->getTemplateSpecializationArgs()) {
                    std::vector<bool> func_tparam_matches;
                    func_tparam_matches.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) {
                        func_tparam_matches[i] = (toDo.func_Ttypes[i] == TAL->get(i).getAsType().getCanonicalType().getAsString());
                    }
                    func_tparam_match =
                        TAL->size() == 0 ? true : std::all_of(func_tparam_matches.begin(), func_tparam_matches.end(), [](bool v) { return v; });
                } else {
                    func_tparam_match = true;
                }
                std::vector<bool> params_matches(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    params_matches[std::distance(params.begin(), it)] =
                        ((*it)->getOriginalType().getCanonicalType().getAsString() ==
                         toDo.params[std::distance(params.begin(), it)].getCanonicalType().getAsString());
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
                if(not(toDo.func_name == FS->getName()) or not(toDo.params.size() == params.size())) {
                    it++;
                    continue;
                }
                std::vector<bool> params_match(params.size());
                for(auto it = params.begin(); it != params.end(); it++) {
                    if(not(*(*it)->getOriginalType()).isDependentType()) { /*Parameter is not a dependent
                                                                              type
                                                                              -> check for exact matching.*/
                        params_match[std::distance(params.begin(), it)] =
                            ((*it)->getOriginalType().getCanonicalType().getAsString() ==
                             toDo.params[std::distance(params.begin(), it)].getCanonicalType().getAsString());
                    } else { /*Parameter is a dependent type -> check for cvr and
                                reference matching.*/
                        params_match[std::distance(params.begin(), it)] =
                            (((*it)->getOriginalType().getTypePtr()->isReferenceType() ==
                              toDo.params[std::distance(params.begin(), it)].getTypePtr()->isReferenceType()) and
                             ((*it)->getOriginalType().getNonReferenceType().getQualifiers() ==
                              toDo.params[std::distance(params.begin(), it)].getNonReferenceType().getQualifiers()));
                    }
                }
                if(std::all_of(params_match.begin(), params_match.end(), [](bool v) { return v; })) {
                    // std::cout
                    //     << "Match!!! Call the rewriter and delete entry from
                    //     toDoList."
                    //     << std::endl;
                    rewriter->InsertText(FS->getBodyRBrace().getLocWithOffset(1), llvm::StringRef(it->getInstantiation()), true, true);
                    it = toDoList->erase(it);
                } else {
                    it++;
                }
            }
        }
    }
}
