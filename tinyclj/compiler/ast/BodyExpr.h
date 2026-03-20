#pragma once

#include "Expr.h"

class BodyExpr : public Expr {
    std::vector<AExpr> m_Exprs;

public:
    void emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    BodyExpr(std::vector<AExpr> exprs);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};