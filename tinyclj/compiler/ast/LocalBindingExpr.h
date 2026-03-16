#pragma once

#include <optional>

#include "UnevaluatableExpr.h"

class LocalBindingExpr : public UnevaluatableExpr {
    std::string m_Value;
    std::optional<int> m_ClosureEnvIndex;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    LocalBindingExpr(std::string value);
};
