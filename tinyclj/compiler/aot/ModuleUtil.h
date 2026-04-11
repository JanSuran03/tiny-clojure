#pragma once

#include <string>
#include <unordered_map>

#include "llvm/IR/Module.h"

#include "compiler/CodegenContext.h"

namespace ModuleUtil {
    std::unordered_map<std::string, llvm::GlobalVariable *>
    declareReferencedGlobals(CodegenContext &ctx, const std::unordered_set<std::string> &global_names);

    llvm::Function *initReferencedGlobals(
            CodegenContext &ctx,
            const std::string &module_name,
            const std::unordered_map<std::string, llvm::GlobalVariable *> &global_vars);

}