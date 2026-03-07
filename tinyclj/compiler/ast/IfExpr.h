#pragma once

#include "Expr.h"

class IfExpr : public Expr {
    AExpr m_CondExpr;
    AExpr m_ThenExpr;
    AExpr m_ElseExpr;

public:
    void emitIR(ExpressionMode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    IfExpr(AExpr condExpr, AExpr thenExpr, AExpr elseExpr);
};