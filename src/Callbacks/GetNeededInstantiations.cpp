#include "Callbacks/GetNeededInstantiations.hpp"

#include "Injection.hpp"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallString.h"

#include <optional>

void GetNeededInstantiations::run(const clang::ast_matchers::MatchFinder::MatchResult& Result)
{
    clang::PrintingPolicy pp(Result.Context->getLangOpts());
    pp.PrintInjectedClassNameWithArguments = true;
    pp.PrintCanonicalTypes = true;
    pp.SuppressDefaultTemplateArgs = true;
    pp.FullyQualifiedName = true;
    pp.SuppressScope = false;
    // pp.UsePreferredNames = true;

    Injection toDo;
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("templ_func_instantation")) {
        if(const clang::MemberSpecializationInfo* MSI = MFS->getMemberSpecializationInfo()) {
            if(MSI->getPointOfInstantiation().isValid()) {
                toDo = Injection::createFromMFS(MFS, pp);
                toDoList->push_back(toDo);
            }
        } else if(const clang::FunctionTemplateSpecializationInfo* TSI = MFS->getTemplateSpecializationInfo()) {
            if(TSI->getPointOfInstantiation().isValid()) {
                toDo = Injection::createFromMFS(MFS, pp);
                toDoList->push_back(toDo);
            }
        }
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("templ_func_instantation")) {
        if(const clang::FunctionTemplateSpecializationInfo* TSI = FS->getTemplateSpecializationInfo()) {
            if(TSI->getPointOfInstantiation().isValid()) {
                // std::cout << termcolor::green << "Created with success." << termcolor::reset << std::endl;
                toDo = Injection::createFromFS(FS, pp);
                toDoList->push_back(toDo);
            }
        }
    }
}
