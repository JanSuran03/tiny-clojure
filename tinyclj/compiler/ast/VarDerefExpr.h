#pragma once

#include "UnevaluatableExpr.h"

class VarDerefExpr : public Expr {
    Object *m_Var;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    VarDerefExpr(Object *var);
};
