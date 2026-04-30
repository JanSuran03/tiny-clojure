#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class VarLiteralExpr : public Expr {
    const Object *m_Var;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    VarLiteralExpr(const Object *var,
                   const std::string &varName);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
