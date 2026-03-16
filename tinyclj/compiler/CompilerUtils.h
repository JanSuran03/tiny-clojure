#pragma once

#include <vector>

#include "compiler/ast/Expr.h"
#include "types/TCVar.h"

namespace CompilerUtils {
    void emitBody(const std::vector<AExpr> &body,
                  const std::string &bodyPrefix,
                  ExpressionMode mode,
                  llvm::AllocaInst *dst,
                  CompilerContext &ctx);

    /** Emit the pointer to the given Var as an LLVM value. */
    llvm::Value *emitVarPtr(TCVar *var, CompilerContext &ctx);
}
