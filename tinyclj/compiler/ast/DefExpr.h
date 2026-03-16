#pragma once

#include "Expr.h"

class DefExpr : public Expr {
    std::string m_Name;
    AExpr m_Value;
public:
    DefExpr(std::string name, AExpr value);

    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object * eval(Runtime &runtime) const override;

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
