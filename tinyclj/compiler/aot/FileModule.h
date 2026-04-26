#pragma once

#include <string>
#include <vector>

#include "Module.h"
#include "compiler/ast/Expr.h"

struct FileModule : public Module {
    FileModule(std::string name,
               std::unordered_set<std::string> imports);
};
