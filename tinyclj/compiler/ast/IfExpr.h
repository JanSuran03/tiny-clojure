#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class IfExpr : public Expr {
    AExpr m_CondExpr;
    AExpr m_ThenExpr;
    AExpr m_ElseExpr;
    LocalVarExpr m_CondLocalVar; // used to store the condition result (todo: Phi nodes could be used to avoid this)

public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    IfExpr(AExpr condExpr,
           AExpr thenExpr,
           AExpr elseExpr,
           LocalVarExpr condLocalVar);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};