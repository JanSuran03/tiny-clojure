#pragma once

#include "Expr.h"
#include "tcdef.h"

class InvokeExpr : public Expr {
    AExpr m_InvokeTarget;
    std::vector<AExpr> m_InvokeArgs;
public:
    void emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    InvokeExpr(AExpr invokeTarget, std::vector<AExpr> invokeArgs);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
