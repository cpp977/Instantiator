#ifndef ASTBUILDER_ACTION_HPP_
#define ASTBUILDER_ACTION_HPP_
#include <iostream>
#include <memory>
#include <vector>

#include "indicators/progress_bar.hpp"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/Tooling.h"

// forward declaration
namespace clang {
class FileManager;
class DiagnosticConsumer;
} // namespace clang

/**
 * \brief Action to compute all ASTs
 *
 * This action is currently running at startup to compute the AST of all files in compile_commands.json.
 * \todo
 * Better to only compute the AST of files only when they are requested.
 */
class AllASTBuilderAction : public clang::tooling::ToolAction
{
    std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs;
    indicators::ProgressBar& prog_bar;

public:
    AllASTBuilderAction(std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs, indicators::ProgressBar& bar)
        : ASTs(ASTs)
        , prog_bar(bar)
    {}

    bool runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                       clang::FileManager* Files,
                       std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                       clang::DiagnosticConsumer* DiagConsumer) override;
};

#endif
