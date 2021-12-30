
int parseOrLoadAST(std::unique_ptr<clang::ASTUnit>& AST, const clang::tooling::CompilationDatabase& db, const std::string filename);

namespace internal {
bool is_cached(const clang::tooling::CompilationDatabase& db, std::string filename);
}
