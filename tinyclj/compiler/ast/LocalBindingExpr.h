#pragma once

#include "UnevaluatableExpr.h"

class LocalBindingExpr : public UnevaluatableExpr {
    std::string m_Value;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    LocalBindingExpr(std::string value);
};
