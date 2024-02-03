#ifndef DEPENDENCIES_H_
#define DEPENDENCIES_H_

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/Utils.h"

/**
 * \brief This class sets the options for the collection of dependencies.
 *
 * System headers are not collected.
 */
class myDependencyCollector : public clang::DependencyCollector
{
public:
    bool sawDependency(llvm::StringRef Filename, bool FromModule, bool IsSystem, bool IsModuleFile, bool IsMissing) override;
    bool needSystemDependencies() override;
};

/**
 * \brief Action to compute the (header) dependencies of a source file.
 *
 * This action uses the compilers `-MM` option to get all dependencies of a source file.
 */
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
