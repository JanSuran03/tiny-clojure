#pragma once

#include "compiler/ast/Expr.h"
#include "types/Object.h"

namespace SemanticAnalyzer {
    bool isSpecial(const Object *obj);

    AExpr analyze(CompilerContext &ctx, const Object *form);

    AExpr analyze(ExpressionMode mode, CompilerContext &ctx, const Object *form);

    Object *macroexpand1(Runtime &rt, const Object *form);

    Object *macroexpand(Runtime &rt, const Object *form);
}
