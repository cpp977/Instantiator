#include <chrono>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <regex>

#include "clang/Tooling/Tooling.h"

#include "Actions/ASTBuilderAction.hpp"
#include "Actions/DependencyAction.hpp"

int parseOrLoadAST(std::unique_ptr<clang::ASTUnit>& AST, const clang::tooling::CompilationDatabase& db, const std::string filename)
{
    clang::tooling::ClangTool Tool(db, filename);
    ASTBuilderAction ast_build_action(AST, db, filename);
    return Tool.run(&ast_build_action);
}

namespace internal {

bool is_cached(const clang::tooling::CompilationDatabase& db, std::string filename)
{
    auto to_time_t = [](auto tp) -> std::time_t {
        auto sctp =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - decltype(tp)::clock::now() + std::chrono::system_clock::now());
        return std::chrono::system_clock::to_time_t(sctp);
    };

    auto print_last_write_time = [to_time_t](std::filesystem::file_time_type const& ftime, const std::string& text) {
        std::cout << text << ": " << ftime.time_since_epoch().count() << std::endl;
    };

    std::filesystem::path p(filename);

    clang::tooling::ClangTool Tool(db, filename);
    std::vector<std::string> deps;
    auto dep_action = std::make_unique<DependencyAction>();
    dep_action->dependencies = &deps;
    class SingleFrontendActionFactory : public clang::tooling::FrontendActionFactory
    {
        std::unique_ptr<DependencyAction> Action;

    public:
        SingleFrontendActionFactory(std::unique_ptr<DependencyAction> Action)
            : Action(std::move(Action))
        {}

        std::unique_ptr<clang::FrontendAction> create() override { return std::move(Action); }
    };

    auto wrapper = std::make_unique<SingleFrontendActionFactory>(std::move(dep_action));
    [[maybe_unused]] int result = Tool.run(wrapper.get());

    if(not std::filesystem::exists(p.replace_extension("ast"))) { return false; }
    auto time_point_ast = std::filesystem::last_write_time(p.replace_extension(std::filesystem::path("ast")));
    for(const auto& dep : deps) {
        std::filesystem::path p_dep(dep);
        auto time_point_cpp = std::filesystem::last_write_time(p_dep);
        // print_last_write_time(time_point_cpp, trim(deps[i]) + " timepoint cpp");
        if(time_point_ast <= time_point_cpp) { return false; }
    }
    return true;
}
} // namespace internal
