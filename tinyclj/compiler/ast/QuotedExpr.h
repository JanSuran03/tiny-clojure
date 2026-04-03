#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class QuotedExpr : public Expr {
    const Object *m_QuotedValue;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    QuotedExpr(const Object *quotedValue);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
