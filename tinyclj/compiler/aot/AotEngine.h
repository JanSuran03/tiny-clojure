#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

#include <llvm/Passes/OptimizationLevel.h>

class AotEngine {
private:
    std::unordered_set<std::string> m_LoadedFiles;
    std::unordered_set<std::string> m_LoadingSet;
    std::vector<std::string> m_LoadingStack;
    std::filesystem::path m_SourceDir = std::filesystem::path(TINYCLJ_PROJECT_SOURCE_DIR) / "clj";
    std::filesystem::path m_CompiledDir = std::filesystem::path(TINYCLJ_PROJECT_SOURCE_DIR) / "compiled";
public:
    AotEngine();

    llvm::OptimizationLevel m_OptimizationLevel = llvm::OptimizationLevel::O0;

    std::filesystem::path fullSourcePath(const std::string &moduleName) const;

    std::filesystem::path fullCompiledPath(const std::string &moduleName, bool optimized = false) const;

    std::filesystem::path fullCompiledDebugPath(const std::string &moduleName, bool optimized = false) const;

    std::filesystem::path fullDepsFile(const std::string &moduleName) const;

    std::filesystem::path compileModule(const std::string &moduleName, bool forceRecompile = false);

    void startLoading(const std::string &moduleName);

    void finishLoading(const std::string &moduleName);

    void loadCompiledModule(const std::string &moduleName, bool forceReload = false);

    bool setCompiledDir(const std::string &path);

    std::unordered_set<std::string> loadedFiles() const;

    std::unordered_set<std::string> loadingSet() const;

    std::vector<std::string> loadingStack() const;

    std::filesystem::path getCompiledDir();
};
