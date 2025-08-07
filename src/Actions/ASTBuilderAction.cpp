#include "Actions/ASTBuilderAction.hpp"

#include <filesystem>

#include "fmt/std.h"
#include "spdlog/spdlog.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/FileManager.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "ASTCreation.hpp"

bool ASTBuilderAction::runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                                     clang::FileManager* Files,
                                     std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                                     clang::DiagnosticConsumer* DiagConsumer)
{
    spdlog::debug("Processing {}", file);
    bool is_cached_on_disk = internal::is_cached(db, file, tmpdir);
    spdlog::debug("cached={}", is_cached_on_disk);
    auto diag_ids = llvm::makeIntrusiveRefCnt<clang::DiagnosticIDs>();
    auto diag_engine = clang::DiagnosticsEngine(diag_ids, Invocation->DiagnosticOpts);

    if(is_cached_on_disk) {
        clang::CompilerInstance CI;
        CI.setInvocation(Invocation);
        CI.createDiagnostics(*createVFSFromCompilerInvocation(*Invocation, diag_engine), DiagConsumer, /*ShouldOwnClient=*/false);
        clang::DiagnosticsEngine* DiagEngine = &CI.getDiagnostics();
#if INSTANTIATOR_LLVM_MAJOR > 16
        AST = clang::ASTUnit::LoadFromASTFile((tmpdir / file.filename().replace_extension("ast")).string(),
                                              CI.getPCHContainerReader(),
                                              clang::ASTUnit::WhatToLoad::LoadEverything,
                                              llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>(DiagEngine),
                                              CI.getFileSystemOpts(),
                                              CI.getHeaderSearchOptsPtr());
#else
        AST = clang::ASTUnit::LoadFromASTFile((tmpdir / file.filename().replace_extension("ast")).string(),
                                              CI.getPCHContainerReader(),
                                              clang::ASTUnit::WhatToLoad::LoadEverything,
                                              llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>(DiagEngine),
                                              CI.getFileSystemOpts());
#endif
    } else {

        AST = clang::ASTUnit::LoadFromCompilerInvocation(
            Invocation,
            std::move(PCHContainerOps),
            clang::CompilerInstance::createDiagnostics(*createVFSFromCompilerInvocation(*Invocation, diag_engine),
                                                       &Invocation->getDiagnosticOpts(),
                                                       DiagConsumer,
                                                       /*ShouldOwnClient=*/false),
            Files);
        AST->Save((tmpdir / file.filename().replace_extension("ast")).string());
    }
    if(!AST or AST->getDiagnostics().hasUncompilableErrorOccurred()) return false;
    return true;
}
