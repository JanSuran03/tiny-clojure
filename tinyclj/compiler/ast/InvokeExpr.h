#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "tcdef.h"

class InvokeExpr : public Expr {
    AExpr m_InvokeTarget;
    std::vector<AExpr> m_InvokeArgs;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    InvokeExpr(AExpr invokeTarget,
               std::vector<AExpr> invokeArgs);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
