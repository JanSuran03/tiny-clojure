#pragma once

#include "Expr.h"
#include "../../types/Object.h"

class DefExpr : public Expr {
    std::string m_Name;
    AExpr m_Value;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    static AExpr parse(Object *form);
};
