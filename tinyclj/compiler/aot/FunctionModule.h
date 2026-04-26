#pragma once

#include <string>
#include <vector>

#include "Module.h"
#include "compiler/ast/Expr.h"

struct FunctionModule : public Module {
    std::string m_VtableName = m_Name + "__vtable";

    FunctionModule(std::string name,
                   std::unordered_set<std::string> imports);

    void writeModule(CodegenContext &ctx);

    void createModuleVtable(CodegenContext &ctx, llvm::Function *callFn);
};
