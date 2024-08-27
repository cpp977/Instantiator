#include "Template.hpp"

#include <cstddef>
#include <llvm/Support/Casting.h>

#include "spdlog/spdlog.h"

#include "clang/AST/DeclTemplate.h"

#include "Injection.hpp"
#include "Parsing.hpp"
#include "TemplateArgument.hpp"

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
    spdlog::debug("Loaded candidate (wo params): {} with #{} parameters.", candidate, parms.size());
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
    if(const auto CTPS = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(MFS->getParent())) {
        auto targs = &CTPS->getTemplateArgs();
        candidate.class_Targs.reserve(targs->size());
        for(std::size_t i = 0; i < targs->size(); ++i) {
            candidate.class_Targs.push_back(Instantiator::TemplateArgument::createFromTemplateArgument(&targs->get(i), pp));
        }
    } else if(auto CTD = MFS->getParent()->getDescribedClassTemplate()) {
        auto ttypes = CTD->getTemplateParameters();
        candidate.class_Targs.resize(ttypes->size());
        for(std::size_t i = 0; i < ttypes->size(); ++i) {
            candidate.class_Targs[i].is_dependent = true;
            candidate.class_Targs[i].names[0] = "";
            if(auto ttype_decl = llvm::dyn_cast<clang::TemplateTypeParmDecl>(ttypes->getParam(i))) {
                if(ttype_decl->isParameterPack()) {
                    candidate.class_Targs[i].kind = Instantiator::Kind::Pack;
                    continue;
                }
                candidate.class_Targs[i].kind = Instantiator::Kind::Type;
            } else if(auto tnontype_decl = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(ttypes->getParam(i))) {
                if(tnontype_decl->isParameterPack()) {
                    candidate.class_Targs[i].kind = Instantiator::Kind::Pack;
                    continue;
                }
                candidate.class_Targs[i].kind = Instantiator::Kind::NonType;
            } else if(auto ttemplate_decl = llvm::dyn_cast<clang::TemplateTemplateParmDecl>(ttypes->getParam(i))) {
                if(ttemplate_decl->isParameterPack()) {
                    candidate.class_Targs[i].kind = Instantiator::Kind::Pack;
                    continue;
                }
                candidate.class_Targs[i].kind = Instantiator::Kind::Template;
            }
        }
    }

    return candidate;
}

bool Template::isTemplateFor(const Injection& toDo) const
{
    spdlog::debug("Calling isTemplateFor between\n\t{}\n\t{}", *this, toDo);
    if(is_constructor != toDo.is_constructor) { return false; }
    if(not(is_constructor) and func_name != toDo.func_name) { return false; }
    if(nested_namespace != toDo.nested_namespace) { return false; }
    if(params.size() != toDo.nonresolved_params.size()) { return false; }
    if(class_name != toDo.class_name) { return false; }
    if(is_const != toDo.is_const) { return false; }
    if(params != toDo.nonresolved_params) { return false; }
    if(class_Targs != toDo.class_Targs) { return false; }
    return true;
}
