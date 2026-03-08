#pragma once

#include "UnevaluatableExpr.h"

// todo: implement eval
class VarExpr : public UnevaluatableExpr {
    std::string m_Value;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    VarExpr(std::string value);

    static AExpr resolveVar(CompilerContext &ctx, const Object *symbol);
};
