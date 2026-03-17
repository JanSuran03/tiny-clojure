#pragma once

#include <vector>
#include <tuple>

#include "UnevaluatableExpr.h"

class RecurExpr : public UnevaluatableExpr {
    std::vector<AExpr> m_RecurArgs;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    RecurExpr(std::vector<AExpr> recurArgs);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
