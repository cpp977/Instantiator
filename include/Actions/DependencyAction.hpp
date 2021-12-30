#ifndef DEPENDENCIES_H_
#define DEPENDENCIES_H_

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/Utils.h"

class myDependencyCollector : public clang::DependencyCollector
{
public:
    bool sawDependency(llvm::StringRef Filename, bool FromModule, bool IsSystem, bool IsModuleFile, bool IsMissing) override;
    bool needSystemDependencies();
};

class DependencyAction : public clang::PreprocessOnlyAction
{
private:
    std::unique_ptr<myDependencyCollector> col;

public:
    std::vector<std::string>* dependencies;

    virtual bool usesPreprocessOnly() const;
    bool BeginSourceFileAction(clang::CompilerInstance& ci);

    virtual void EndSourceFileAction();
};

#endif
