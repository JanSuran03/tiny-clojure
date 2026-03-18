#pragma once

#include "UnevaluatableExpr.h"

class VarDerefExpr : public Expr {
    Object *m_Var;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object * eval(Runtime &runtime) const override;

    VarDerefExpr(Object *var);
};
