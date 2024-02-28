#include "Actions/ASTBuilderAction.hpp"

#include <filesystem>
#include <iostream>

#include "ASTCreation.hpp"

#include "clang/Basic/FileManager.h"

bool ASTBuilderAction::runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                                     clang::FileManager* Files,
                                     std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                                     clang::DiagnosticConsumer* DiagConsumer)
{
  bool is_cached_on_disk = internal::is_cached(db, file, tmpdir);
    // std::cout << "Processing " << filename << std::endl;
    // std::cout << std::boolalpha << "cached=" << is_cached_on_disk << std::endl;
    if(is_cached_on_disk) {
        clang::CompilerInstance CI;
        CI.setInvocation(Invocation);
        CI.createDiagnostics(DiagConsumer, /*ShouldOwnClient=*/false);
        clang::DiagnosticsEngine* DiagEngine = &CI.getDiagnostics();
        AST = clang::ASTUnit::LoadFromASTFile((tmpdir / file.filename().replace_extension("ast")).string(),
                                              CI.getPCHContainerReader(),
                                              clang::ASTUnit::WhatToLoad::LoadEverything,
                                              llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>(DiagEngine),
                                              CI.getFileSystemOpts());
    } else {
        AST = clang::ASTUnit::LoadFromCompilerInvocation(Invocation,
                                                         std::move(PCHContainerOps),
                                                         clang::CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts(),
                                                                                                    DiagConsumer,
                                                                                                    /*ShouldOwnClient=*/false),
                                                         Files);
        AST->Save((tmpdir / file.filename().replace_extension("ast")).string());
    }
    if(!AST or AST->getDiagnostics().hasUncompilableErrorOccurred()) return false;
    return true;
}
