#include "Parsing.hpp"

#include "spdlog/spdlog.h"

#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateName.h"

namespace internal {
std::vector<std::string> parseTemplateArgs(const clang::TemplateArgumentList* TAL, clang::PrintingPolicy pp)
{
    std::vector<std::string> out;
    if(TAL) {
        out.resize(TAL->size());
        for(std::size_t i = 0; i < TAL->size(); i++) {
            if(TAL->get(i).isDependent()) { spdlog::info("Dependent template argument."); }
            switch(TAL->get(i).getKind()) {
            case clang::TemplateArgument::ArgKind::Type: {
                out[i] = TAL->get(i).getAsType().getAsString(pp);
                spdlog::info("type: {}", out[i]);
                break;
            }
            case clang::TemplateArgument::ArgKind::Integral: {
                llvm::SmallString<10> name;
                TAL->get(i).getAsIntegral().toString(name);
                out[i] = name.str().str();
                spdlog::info("integral: {}", out[i]);
                break;
            }
            case clang::TemplateArgument::ArgKind::Pack: {
                out.resize(out.size() + TAL->get(i).pack_size() - 1);
                for(auto pack_it = TAL->get(i).pack_begin(); pack_it != TAL->get(i).pack_end(); pack_it++) {
                    switch(pack_it->getKind()) {
                    case clang::TemplateArgument::ArgKind::Type: {
                        out[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = pack_it->getAsType().getAsString(pp);
                        break;
                    }
                    case clang::TemplateArgument::ArgKind::Integral: {
                        llvm::SmallString<10> name;
                        pack_it->getAsIntegral().toString(name);
                        out[i + std::distance(TAL->get(i).pack_begin(), pack_it)] = name.str().str();
                        break;
                    }
                    default: {
                        break;
                    }
                    }
                }
                break;
            }
            case clang::TemplateArgument::ArgKind::Template: {
                llvm::raw_string_ostream OS(out[i]);
#if INSTANTIATOR_LLVM_MAJOR > 13
                TAL->get(i).getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::Fully);
#else
                TAL->get(i).getAsTemplate().print(OS, pp, false);
#endif
                OS.str();
                break;
            }
            default: {
                break;
            }
            }
        }
    }
    return out;
}

std::vector<Param> parseFunctionArgs(std::vector<clang::ParmVarDecl*> Args, clang::PrintingPolicy pp)
{
    std::vector<Param> out;
    out.resize(Args.size());
    for(auto it = Args.begin(); it != Args.end(); it++) { out[std::distance(Args.begin(), it)] = Param::createFromParmVarDecl(*it, pp); }
    return out;
}
} // namespace internal
