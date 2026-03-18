#pragma once

#include "Expr.h"

class VarLiteralExpr : public Expr {
    Object *m_Var;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object * eval(Runtime &runtime) const override;

    VarLiteralExpr(Object *var);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
