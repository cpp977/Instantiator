#ifndef ASTBUILDER_ACTION_HPP_
#define ASTBUILDER_ACTION_HPP_
#include <iostream>
#include <memory>
#include <vector>

#include "indicators/progress_bar.hpp"

#include "clang/Basic/FileManager.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/Tooling.h"

class ASTBuilderAction : public clang::tooling::ToolAction
{
    std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs;
    indicators::ProgressBar& prog_bar;
public:
    ASTBuilderAction(std::vector<std::unique_ptr<clang::ASTUnit>>& ASTs, indicators::ProgressBar& bar)
        : ASTs(ASTs), prog_bar(bar)
    {}

    bool runInvocation(std::shared_ptr<clang::CompilerInvocation> Invocation,
                       clang::FileManager* Files,
                       std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                       clang::DiagnosticConsumer* DiagConsumer) override
    {
        std::unique_ptr<clang::ASTUnit> AST =
            clang::ASTUnit::LoadFromCompilerInvocation(Invocation,
                                                       std::move(PCHContainerOps),
                                                       clang::CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts(),
                                                                                                  DiagConsumer,
                                                                                                  /*ShouldOwnClient=*/false),
                                                       Files);
        if(!AST or AST->getDiagnostics().hasUncompilableErrorOccurred()) return false;
        prog_bar.tick();
        prog_bar.set_option(indicators::option::PostfixText{"Processing: "+AST->getOriginalSourceFileName().str()});  

        // std::cout << "Parsed AST for: " << AST->getOriginalSourceFileName().str() << std::endl;
        ASTs.push_back(std::move(AST));
        return true;
    }
};

#endif
