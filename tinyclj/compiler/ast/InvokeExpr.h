#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "tcdef.h"

class InvokeExpr : public Expr {
    AExpr m_InvokeTarget;
    std::vector<AExpr> m_InvokeArgs;
    /// The index of the invoke expression in the current function frame, used for acquiring the
    /// reserved stack space for packing the native call arguments on the caller side into argv.
    size_t m_InvokeIndex;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    InvokeExpr(AExpr invokeTarget,
               std::vector<AExpr> invokeArgs,
               size_t invokeIndex);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
