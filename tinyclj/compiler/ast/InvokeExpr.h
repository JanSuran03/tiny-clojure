#pragma once

#include "local-binding/LocalVarExpr.h"
#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "tcdef.h"

class InvokeExpr : public Expr {
    struct NativeInvokeArgArray {
        unsigned m_FunctionLocalsOffset;
        unsigned m_Length;
    };

    AExpr m_InvokeTarget;
    std::vector<AExpr> m_InvokeArgs;
    LocalVarExpr m_TargetResLocalVar;
    std::vector<LocalVarExpr> m_InvokeArgsLocalsVars;
    NativeInvokeArgArray m_PackedArgArrayLocalVar;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    InvokeExpr(AExpr invokeTarget,
               std::vector<AExpr> invokeArgs,
               LocalVarExpr targetResLocalVar,
               std::vector<LocalVarExpr> invokeArgsLocalVars,
               NativeInvokeArgArray packedArgArrayLocalVar);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
