#pragma once

#include "AnalyzerContext.h"
#include "compiler/ast/Expr.h"
#include "types/Object.h"

namespace SemanticAnalyzer {
    bool isSpecial(const Object *obj);

    AExpr analyze(AnalyzerContext &ctx, const Object *form);

    AExpr analyze(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);

    Object *macroexpand1(Runtime &rt, const Object *form);

    Object *macroexpand(Runtime &rt, const Object *form);
}
