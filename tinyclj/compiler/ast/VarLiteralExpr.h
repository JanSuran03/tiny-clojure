#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class VarLiteralExpr : public Expr {
    Object *m_Var;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    VarLiteralExpr(Object *var);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
