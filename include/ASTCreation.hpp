#ifndef AST_CREATION_H_
#define AST_CREATION_H_

#include "clang/Tooling/Tooling.h"
#include <filesystem>

int parseOrLoadAST(std::unique_ptr<clang::ASTUnit>& AST,
                   const clang::tooling::CompilationDatabase& db,
                   const std::filesystem::path& filename,
                   const std::filesystem::path& tmpdir);

namespace internal {
/**
 * Helper function to detect whether the AST for \p filename is cached on disk.
 * \return `true` if the AST is cached, `false` if not.
 *
 * \param db : [clang::tooling::CompilationDatabase](https://clang.llvm.org/doxygen/classclang_1_1tooling_1_1CompilationDatabase.html) to be able to
 *             invoke the compiler for getting the dependencies.
 * \param filename : Name of the source file
 */
bool is_cached(const clang::tooling::CompilationDatabase& db, const std::filesystem::path& filename, const std::filesystem::path& tmpdir);
} // namespace internal

#endif
