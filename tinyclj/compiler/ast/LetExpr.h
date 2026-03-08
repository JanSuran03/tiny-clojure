#pragma once

#include <vector>
#include <tuple>

#include "UnevaluatableExpr.h"

class LetExpr : public UnevaluatableExpr {
    std::vector<std::tuple<std::string, AExpr>> m_Bindings;
    AExpr m_Body;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, AExpr body);

    static AExpr parse(CompilerContext &ctx, const Object *form);
};
