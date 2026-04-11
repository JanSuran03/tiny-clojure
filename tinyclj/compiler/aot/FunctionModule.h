#pragma once

#include <string>
#include <vector>

#include "compiler/ast/Expr.h"

struct FunctionModule {
    std::string m_Name;
    std::unordered_set<std::string> m_Imports;

    FunctionModule(std::string name, std::unordered_set<std::string> imports);

    void emitImportsFile();

    void writeModule(CodegenContext &ctx);
};
