#pragma once

#include <string>

namespace ModuleLoader {
    void loadCompiledModule(const std::string &moduleName, bool forceReload, bool isMainModule);
}