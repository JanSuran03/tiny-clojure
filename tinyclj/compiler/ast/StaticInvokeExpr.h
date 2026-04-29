#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "tcdef.h"

class StaticInvokeExpr : public Expr {
    const Object *m_NativeFunctionObject;
    std::vector<AExpr> m_InvokeArgs;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    StaticInvokeExpr(const Object *nativeFunctionObject, std::vector<AExpr> invokeArgs);
};
