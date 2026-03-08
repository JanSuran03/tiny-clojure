#pragma once

#include "Expr.h"

class VarExpr : public Expr {
    std::string m_Value;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    VarExpr(std::string value);

    static AExpr resolveVar(CompilerContext &ctx, const std::string &name);
};
