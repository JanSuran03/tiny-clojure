#pragma once

#include "Expr.h"

class BodyExpr : public Expr {
    std::vector<AExpr> m_Exprs;

public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    BodyExpr(std::vector<AExpr> exprs);
};