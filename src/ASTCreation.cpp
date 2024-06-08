#include <chrono>
#include <filesystem>
#include <memory>
#include <stdexcept>

#include "ASTCreation.hpp"

#include "Actions/ASTBuilderAction.hpp"
#include "Actions/DependencyAction.hpp"

void parseOrLoadAST(std::unique_ptr<clang::ASTUnit>& AST,
                    const clang::tooling::CompilationDatabase& db,
                    const std::filesystem::path& filename,
                    const std::filesystem::path& tmpdir)
{
    clang::tooling::ClangTool Tool(db, filename.string());
    ASTBuilderAction ast_build_action(AST, db, filename, tmpdir);
    int success = Tool.run(&ast_build_action);
    if(success == 1) {
        throw std::runtime_error("Error while creating the AST for " + filename.string() +
                                 ". Check for issing includes due to generated instantiations.");
    }
}

namespace internal {

bool is_cached(const clang::tooling::CompilationDatabase& db, const std::filesystem::path& file, const std::filesystem::path& tmpdir)
{
    // auto to_time_t = [](auto tp) -> std::time_t {
    //     auto sctp =
    //         std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - decltype(tp)::clock::now() + std::chrono::system_clock::now());
    //     return std::chrono::system_clock::to_time_t(sctp);
    // };

    // auto print_last_write_time = [](std::filesystem::file_time_type const& ftime, const std::string& text) {
    //     std::cout << text << ": " << ftime.time_since_epoch().count() << std::endl;
    // };

    clang::tooling::ClangTool Tool(db, file.string());
    std::vector<std::string> deps;
    class SingleFrontendActionFactory : public clang::tooling::FrontendActionFactory
    {
        std::vector<std::string>* deps;

    public:
        SingleFrontendActionFactory(std::vector<std::string>* deps)
            : deps(deps)
        {}

        std::unique_ptr<clang::FrontendAction> create() override
        {
            auto dep_action = std::make_unique<DependencyAction>();
            dep_action->dependencies = deps;
            return std::move(dep_action);
        }
    };

    auto wrapper = std::make_unique<SingleFrontendActionFactory>(&deps);
    [[maybe_unused]] int result = Tool.run(wrapper.get());

    if(not std::filesystem::exists(tmpdir / file.filename().replace_extension("ast"))) { return false; }
    auto time_point_ast = std::filesystem::last_write_time(tmpdir / file.filename().replace_extension("ast"));
    for(const auto& dep : deps) {
        std::filesystem::path p_dep(dep);
        auto time_point_cpp = std::filesystem::last_write_time(p_dep);
        // print_last_write_time(time_point_cpp, trim(deps[i]) + " timepoint cpp");
        if(time_point_ast <= time_point_cpp) { return false; }
    }
    return true;
}
} // namespace internal
