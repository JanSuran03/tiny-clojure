#pragma once

#include "compiler/AnalyzerContext.h"
#include "Expr.h"

class QuotedExpr : public Expr {
    const Object *m_QuotedValue;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    QuotedExpr(const Object *quotedValue);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
