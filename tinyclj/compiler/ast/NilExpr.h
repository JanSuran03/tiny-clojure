#pragma once

#include "Expr.h"

class NilExpr : public Expr {
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

public:
    NilExpr() = default;
};
