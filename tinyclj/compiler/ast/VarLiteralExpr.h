#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class VarLiteralExpr : public Expr {
    Object *m_Var;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    VarLiteralExpr(Object *var);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
