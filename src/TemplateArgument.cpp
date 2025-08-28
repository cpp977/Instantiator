#include "TemplateArgument.hpp"

#include <clang/AST/APValue.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TemplateBase.h>
#include <cstddef>

#include "spdlog/spdlog.h"

#include "clang/AST/TemplateName.h"
#include "llvm/ADT/SmallString.h"

namespace Instantiator {

std::string TemplateArgument::name(const std::string& sep) const
{
    std::string out = {};
    for(std::size_t i = 0; i < names.size(); ++i) {
        if(i < names.size() - 1) {
            out += names[i] + sep;
        } else {
            out += names[i];
        }
    }
    return out;
}

TemplateArgument TemplateArgument::createFromTemplateArgument(const clang::TemplateArgument* parm, const clang::PrintingPolicy pp)
{
    TemplateArgument out;

    out.is_dependent = parm->isDependent();

    switch(parm->getKind()) {
    case clang::TemplateArgument::ArgKind::Type: {
        out.names[0] = parm->getAsType().getAsString(pp);
        out.kind = Instantiator::Kind::Type;
        spdlog::debug("type: {}", out.names[0]);
        break;
    }
    case clang::TemplateArgument::ArgKind::Integral: {
        llvm::SmallString<10> tmp_name;
        parm->getAsIntegral().toString(tmp_name);
        out.names[0] = tmp_name.str().str();
        out.kind = Instantiator::Kind::NonType;
        spdlog::debug("integral: {}", out.names[0]);
        break;
    }
    case clang::TemplateArgument::ArgKind::Pack: {
        out.names.resize(parm->pack_size());
        for(auto pack_it = parm->pack_begin(); pack_it != parm->pack_end(); pack_it++) {
            switch(pack_it->getKind()) {
            case clang::TemplateArgument::ArgKind::Type: {
                out.names[std::distance(parm->pack_begin(), pack_it)] = pack_it->getAsType().getAsString(pp);
                break;
            }
            case clang::TemplateArgument::ArgKind::Integral: {
                llvm::SmallString<10> name;
                pack_it->getAsIntegral().toString(name);
                out.names[std::distance(parm->pack_begin(), pack_it)] = name.str().str();
                break;
            }
            default: {
                break;
            }
            }
        }
        out.kind = Instantiator::Kind::Pack;
        break;
    }
    case clang::TemplateArgument::ArgKind::Template: {
        llvm::raw_string_ostream OS(out.names[0]);
#if INSTANTIATOR_LLVM_MAJOR > 13
        parm->getAsTemplate().print(OS, pp, clang::TemplateName::Qualified::AsWritten);
#else
        parm->getAsTemplate().print(OS, pp, false);
#endif
        OS.str();
        out.kind = Instantiator::Kind::Template;
        break;
    }
#if INSTANTIATOR_LLVM_MAJOR > 17
    case clang::TemplateArgument::ArgKind::StructuralValue: {
        spdlog::critical("NTTP are not implemented correctly. Param={}", parm->getAsStructuralValue().isStruct());
        break;
    }
#endif
    case clang::TemplateArgument::ArgKind::Expression: {
        spdlog::critical("Expression TA are not implemented correctly. Param={}", parm->getAsExpr()->getType().getAsString(pp));
        break;
    }
    default: {
        spdlog::critical("Unhandled clang::TemplateArgument::ArgKind={}, isExpression={}",
                         static_cast<int>(parm->getKind()),
                         parm->getKind() == clang::TemplateArgument::ArgKind::Expression);
        break;
    }
    }
    return out;
}

bool TemplateArgument::compare(const TemplateArgument& other) const
{
    spdlog::debug("name dependent={}, other.name dependent={}", is_dependent, other.is_dependent);
    if(is_dependent or other.is_dependent) { return true /*kind == other.kind*/; }
    spdlog::debug("Compare of {} and {}", name(), other.name());
    return name() == other.name();
}

} // namespace Instantiator
