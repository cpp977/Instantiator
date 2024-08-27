#ifndef INSTANTIATOR_TEMPLATE_ARGUMENT_H_
#define INSTANTIATOR_TEMPLATE_ARGUMENT_H_

#include <clang/AST/TemplateBase.h>
#include <string>

#include "fmt/color.h"
#include "fmt/core.h"

#include "clang/AST/PrettyPrinter.h"

namespace Instantiator {

enum class Kind
{
    Type,
    NonType,
    Template,
    Pack
};

/**
 * \brief Struct for the collection of all relevant data for a function parameter.
 */
struct TemplateArgument
{
    /**Kind of the template argument (Type, NonType, ...)*/
    Kind kind{};

    /**Name of the template argument*/
    std::vector<std::string> names = {""};

    std::string name(const std::string& sep = " ") const;

    /**Whether this template argument is dependent (not already resolved).*/
    bool is_dependent = false;

    /**
     * Load the parameter from a [clang::ParmVarDecl](https://clang.llvm.org/doxygen/classclang_1_1ParmVarDecl.html).
     * \param parm Pointer to the `clang::ParmVarDecl`
     * \param pp [clang::PrintingPolicy](https://clang.llvm.org/doxygen/structclang_1_1PrintingPolicy.html) which controls how strings are created.
     */
    static TemplateArgument createFromTemplateArgument(const clang::TemplateArgument* parm, clang::PrintingPolicy pp);

    /**
     * Compares if two parameters are the same.
     */
    bool compare(const TemplateArgument& other) const;

    inline bool operator==(const TemplateArgument& other) const { return this->compare(other); }
};

} // namespace Instantiator

template <>
class fmt::formatter<Instantiator::TemplateArgument>
{
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format(Instantiator::TemplateArgument const& p, Context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", p.is_dependent ? "dependent" : p.name());
    }
};

#endif
