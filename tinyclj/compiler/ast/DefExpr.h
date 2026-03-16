#pragma once

#include "Expr.h"
#include "types/TCVar.h"

class DefExpr : public Expr {
    TCVar *m_Var;
    AExpr m_Value;
public:
    DefExpr(TCVar *var, AExpr value);

    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
