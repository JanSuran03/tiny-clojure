#pragma once

#include "Expr.h"
#include "types/Object.h"

namespace Parser {
    AExpr analyze(CompilerContext &ctx, const Object *form);

    Object *eval(CompilerContext &ctx, const Object *form);
}
