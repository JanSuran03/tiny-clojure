#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class DefExpr : public Expr {
    Object *m_Var;
    AExpr m_Value;
public:
    DefExpr(Object *var, AExpr value);

    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
