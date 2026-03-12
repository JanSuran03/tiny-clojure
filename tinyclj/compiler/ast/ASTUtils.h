#pragma once

#include <vector>

#include "Expr.h"

namespace AstUtils {
    void emitBody(const std::vector<AExpr> &body,
                  const std::string &bodyPrefix,
                  ExpressionMode mode,
                  llvm::AllocaInst *dst,
                  CompilerContext &ctx);
}
