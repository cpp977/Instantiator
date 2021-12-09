#include <optional>

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
    pp.PrintInjectedClassNameWithArguments = true;
    pp.PrintCanonicalTypes = true;
    pp.SuppressDefaultTemplateArgs = true;
    pp.FullyQualifiedName = true;
    pp.SuppressScope = false;
    // pp.UsePreferredNames = true;

    std::optional<Injection> toDo;
    if(const clang::CXXMethodDecl* MFS = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("templ_func_instantation")) {
        toDo = Injection::createFromMFS(MFS, pp);
        if(toDo) toDoList->push_back(toDo.value());
    } else if(const clang::FunctionDecl* FS = Result.Nodes.getNodeAs<clang::FunctionDecl>("templ_func_instantation")) {
        toDo = Injection::createFromFS(FS, pp);
        if(toDo) toDoList->push_back(toDo.value());
    }
}
