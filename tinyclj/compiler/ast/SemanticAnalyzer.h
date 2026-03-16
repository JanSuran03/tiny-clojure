#pragma once

#include "Expr.h"
#include "types/Object.h"

namespace SemanticAnalyzer {
    AExpr analyze(CompilerContext &ctx, const Object *form);

    AExpr analyze(ExpressionMode mode, CompilerContext &ctx, const Object *form);
}
