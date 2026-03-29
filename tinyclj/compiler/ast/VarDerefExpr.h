#pragma once

#include "UnevaluatableExpr.h"

class VarDerefExpr : public Expr {
    Object *m_Var;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    VarDerefExpr(Object *var);
};
