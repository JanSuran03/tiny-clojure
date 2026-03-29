#pragma once

#include "compiler/AnalyzerContext.h"
#include <vector>
#include <tuple>

#include "UnevaluatableExpr.h"

class LetExpr : public UnevaluatableExpr {
    std::vector<std::tuple<std::string, AExpr>> m_Bindings;
    std::vector<AExpr> m_Body;
    bool m_IsLoop;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, std::vector<AExpr> body, bool isLoop);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
