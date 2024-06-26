#ifndef ASTBUILDER_ACTION_HPP_
#define ASTBUILDER_ACTION_HPP_

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/Tooling.h"

#include <filesystem>
#include <memory>

// forward declaration
namespace clang {
class FileManager;
class DiagnosticConsumer;
} // namespace clang

/**
 * \brief Action to compute a single AST of file \p filename.
 *
 * The action first looks if the AST needs to be reparsed or if it can be deserialized from disk.
 */
class ASTBuilderAction : public clang::tooling::ToolAction
{
    std::unique_ptr<clang::ASTUnit>& AST;
    const clang::tooling::CompilationDatabase& db;
    std::filesystem::path file;
    std::filesystem::path tmpdir;

public:
    ASTBuilderAction(std::unique_ptr<clang::ASTUnit>& AST,
                     const clang::tooling::CompilationDatabase& db,
                     const std::filesystem::path& filename,
                     const std::filesystem::path& tmpdir)
        : AST(AST)
        , db(db)
        , file(filename)
        , tmpdir(tmpdir)
    {}

    bool runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                       clang::FileManager* Files,
                       std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                       clang::DiagnosticConsumer* DiagConsumer) override;
};

#endif
