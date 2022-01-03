#ifndef AST_CREATION_H_
#define AST_CREATION_H_

#include "clang/Tooling/Tooling.h"

int parseOrLoadAST(std::unique_ptr<clang::ASTUnit>& AST, const clang::tooling::CompilationDatabase& db, const std::string filename);

namespace internal {
bool is_cached(const clang::tooling::CompilationDatabase& db, std::string filename);
}

#endif
