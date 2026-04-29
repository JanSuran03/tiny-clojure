#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "InvokeExpr.h"
#include "tcdef.h"

class DynamicInvokeExpr : public InvokeExpr {
    AExpr m_InvokeTarget;
protected:
    const Object *evalInvoke(const std::vector<const Object *> &evaled_args) const override;

public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    DynamicInvokeExpr(AExpr invokeTarget, std::vector<AExpr> invokeArgs);
};
