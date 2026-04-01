#pragma once

#include "compiler/AnalyzerContext.h"
#include <vector>
#include <tuple>

#include "UnevaluatableExpr.h"

class RecurExpr : public UnevaluatableExpr {
    std::vector<AExpr> m_RecurArgs;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    RecurExpr(std::vector<AExpr> recurArgs);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
