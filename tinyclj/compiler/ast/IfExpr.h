#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class IfExpr : public Expr {
    AExpr m_CondExpr;
    AExpr m_ThenExpr;
    AExpr m_ElseExpr;

public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    IfExpr(AExpr condExpr,
           AExpr thenExpr,
           AExpr elseExpr);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};