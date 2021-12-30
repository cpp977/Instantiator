#include "Actions/DependencyAction.hpp"

bool myDependencyCollector::sawDependency(llvm::StringRef Filename, bool FromModule, bool IsSystem, bool IsModuleFile, bool IsMissing)
{
    if(IsSystem) {
        return false;
    } else {
        return true;
    }
}
bool myDependencyCollector::needSystemDependencies() { return false; }

bool DependencyAction::usesPreprocessOnly() const { return true; }
bool DependencyAction::BeginSourceFileAction(clang::CompilerInstance& ci)
{
    clang::Preprocessor& pp = ci.getPreprocessor();
    col = std::make_unique<myDependencyCollector>();
    col->attachToPreprocessor(pp);
    return true;
}

void DependencyAction::EndSourceFileAction()
{
    auto deps = col->getDependencies();
    *dependencies = std::vector<std::string>(deps.begin(), deps.end());
}
