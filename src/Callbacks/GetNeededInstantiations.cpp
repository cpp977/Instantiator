#include "Callbacks/GetNeededInstantiations.hpp"

#include "llvm/ADT/APInt.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"

#include "Injection.hpp"

void GetNeededInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    Injection toDo;
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("templ_func_instantation")) {
        toDo.is_nontemplate_member = true;
        toDo.return_type = MFS->getReturnType().getCanonicalType().getAsString();
        toDo.func_name = MFS->getName();
        toDo.class_name = MFS->getParent()->getName();
        if(const clang::ClassTemplateSpecializationDecl* CTS = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(MFS->getParent())) {
            const clang::TemplateArgumentList& TAL = CTS->getTemplateArgs();
            toDo.class_Ttypes.resize(TAL.size());
            for(std::size_t i = 0; i < TAL.size(); i++) { toDo.class_Ttypes[i] = TAL.get(i).getAsType().getCanonicalType().getAsString(); }
        }
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            if(MSI->getPointOfInstantiation().isValid()) {
                if(const clang::TemplateArgumentList* TAL = MFS->getTemplateSpecializationArgs()) {
                    toDo.func_Ttypes.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) { toDo.func_Ttypes[i] = TAL->get(i).getAsType().getCanonicalType().getAsString(); }
                }
                llvm::ArrayRef<clang::ParmVarDecl*> params = MFS->parameters();
                toDo.params.resize(params.size());
                for(auto it = params.begin(); it != params.end(); it++) { toDo.params[std::distance(params.begin(), it)] = (*it)->getOriginalType(); }
                toDo.is_const = MFS->isConst();
                toDoList->push_back(toDo);
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("templ_func_instantation")) {
        toDo.is_nontemplate_member = false;

        if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
            if(TSI->getPointOfInstantiation().isValid()) {
                toDo.func_name = FS->getName();
                toDo.return_type = FS->getReturnType().getCanonicalType().getAsString();
                if(const clang::TemplateArgumentList* TAL = FS->getTemplateSpecializationArgs()) {
                    toDo.func_Ttypes.resize(TAL->size());
                    for(std::size_t i = 0; i < TAL->size(); i++) { toDo.func_Ttypes[i] = TAL->get(i).getAsType().getCanonicalType().getAsString(); }
                }

                llvm::ArrayRef<clang::ParmVarDecl*> params = FS->parameters();
                toDo.params.resize(params.size());
                for(auto it = params.begin(); it != params.end(); it++) { toDo.params[std::distance(params.begin(), it)] = (*it)->getOriginalType(); }
                toDoList->push_back(toDo);
            }
        }
    }
}
