#pragma once

#include <string>
#include <unordered_map>

#include "llvm/IR/Module.h"

#include "compiler/CodegenContext.h"

namespace ModuleUtil {
    void createGlobalsInitFunction(
            CodegenContext &ctx,
            const std::string &module_name,
            const std::unordered_map<std::string, llvm::GlobalVariable *> &global_vars);

}