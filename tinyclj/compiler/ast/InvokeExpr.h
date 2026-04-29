#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "tcdef.h"

class InvokeExpr : public Expr {
protected:
    std::vector<AExpr> m_InvokeArgs;

    virtual const Object *evalInvoke(const std::vector<const Object *> &evaled_args) const = 0;

public:
    const Object *eval() const final;

    InvokeExpr(std::vector<AExpr> invokeArgs);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
