#pragma once

#include <string>
#include <vector>

#include "compiler/ast/Expr.h"

struct Module {
    std::string m_Name;
    std::unordered_set<std::string> m_Imports;

    Module(std::string name,
           std::unordered_set<std::string> imports);

    void emitImportsFile();
};
