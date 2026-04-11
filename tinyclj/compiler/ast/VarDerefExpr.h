#pragma once

#include "UnevaluatableExpr.h"
#include "compiler/AnalyzerContext.h"

class VarDerefExpr : public Expr {
    Object *m_Var;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    VarDerefExpr(Object *var,
                 const std::string &varName,
                 AnalyzerContext &ctx);
};
