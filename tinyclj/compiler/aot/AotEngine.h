#pragma once

#include <string>
#include <unordered_set>
#include <vector>

class AotEngine {
public: //todo
    std::unordered_set<std::string> m_LoadedFiles;
    std::unordered_set<std::string> m_LoadingSet;
    std::vector<std::string> m_LoadingStack;
    std::string m_SourceRoot = std::string(TINYCLJ_PROJECT_SOURCE_DIR) + "/src";
    std::string m_CompiledRoot = std::string(TINYCLJ_PROJECT_SOURCE_DIR) + "/modules";
public:
    std::string fullSourcePath(const std::string &moduleName) const;

    std::string fullCompiledPath(const std::string &moduleName) const;

    std::string fullCompiledDebugPath(const std::string &moduleName) const;

    std::string fullDepsFileName(const std::string &moduleName) const;

    std::string compileModule(const std::string &moduleName, bool forceRecompile = false);

    void startLoading(const std::string &moduleName);

    void finishLoading(const std::string &moduleName);

    void loadCompiledModule(const std::string &moduleName, bool forceReload = false);
};
