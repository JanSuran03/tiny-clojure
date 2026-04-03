#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class BodyExpr : public Expr {
    std::vector<AExpr> m_Exprs;

public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    BodyExpr(std::vector<AExpr> exprs);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};