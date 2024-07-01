#include "Actions/AllASTBuilderAction.hpp"

#include "spdlog/spdlog.h"

#include "clang/Basic/FileManager.h"

bool AllASTBuilderAction::runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                                        clang::FileManager* Files,
                                        std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                                        clang::DiagnosticConsumer* DiagConsumer)
{
    std::unique_ptr<clang::ASTUnit> AST =
        clang::ASTUnit::LoadFromCompilerInvocation(Invocation,
                                                   std::move(PCHContainerOps),
                                                   clang::CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts(),
                                                                                              DiagConsumer,
                                                                                              /*ShouldOwnClient=*/false),
                                                   Files);
    if(!AST or AST->getDiagnostics().hasUncompilableErrorOccurred()) return false;

    spdlog::debug("Parsed AST for {}", AST->getOriginalSourceFileName().str());
    ASTs.push_back(std::move(AST));
    return true;
}
