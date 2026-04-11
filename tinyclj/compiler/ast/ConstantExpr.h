#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class ConstantExpr : public Expr {
    const Object *m_Obj;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    ConstantExpr(const Object *quotedValue);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
