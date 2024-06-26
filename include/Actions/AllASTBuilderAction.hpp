#ifndef ASTBUILDER_ACTION_HPP_
#define ASTBUILDER_ACTION_HPP_

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/Tooling.h"

#include <memory>
#include <vector>

// forward declaration
namespace clang {
class FileManager;
class DiagnosticConsumer;
} // namespace clang

/**
 * \brief Action to compute all ASTs
 *
 * This action is currently not used as it is loading all ASTs from the compilation database.
 */
class AllASTBuilderAction : public clang::tooling::ToolAction
{
    std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs;

public:
    AllASTBuilderAction(std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs)
        : ASTs(ASTs)
    {}

    bool runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                       clang::FileManager* Files,
                       std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                       clang::DiagnosticConsumer* DiagConsumer) override;
};

#endif
