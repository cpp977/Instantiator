#include "Actions/AllASTBuilderAction.hpp"

#include "spdlog/spdlog.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/FileManager.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

bool AllASTBuilderAction::runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                                        clang::FileManager* Files,
                                        std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                                        clang::DiagnosticConsumer* DiagConsumer)
{
    auto diag_ids = llvm::makeIntrusiveRefCnt<clang::DiagnosticIDs>();
    auto diag_engine = clang::DiagnosticsEngine(diag_ids, Invocation->DiagnosticOpts);
    std::unique_ptr<clang::ASTUnit> AST = clang::ASTUnit::LoadFromCompilerInvocation(
        Invocation,
        std::move(PCHContainerOps),
        clang::CompilerInstance::createDiagnostics(*createVFSFromCompilerInvocation(*Invocation, diag_engine),
                                                   &Invocation->getDiagnosticOpts(),
                                                   DiagConsumer,
                                                   /*ShouldOwnClient=*/false),
        Files);
    if(!AST or AST->getDiagnostics().hasUncompilableErrorOccurred()) return false;

    spdlog::debug("Parsed AST for {}", AST->getOriginalSourceFileName().str());
    ASTs.push_back(std::move(AST));
    return true;
}
