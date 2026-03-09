#pragma once

#include <vector>
#include <tuple>

#include "UnevaluatableExpr.h"

class LetExpr : public UnevaluatableExpr {
    std::vector<std::tuple<std::string, AExpr>> m_Bindings;
    std::vector<AExpr> m_Body;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, std::vector<AExpr> body);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
