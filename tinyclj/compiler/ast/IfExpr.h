#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class IfExpr : public Expr {
    AExpr m_CondExpr;
    AExpr m_ThenExpr;
    AExpr m_ElseExpr;

public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    IfExpr(AExpr condExpr, AExpr thenExpr, AExpr elseExpr);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};