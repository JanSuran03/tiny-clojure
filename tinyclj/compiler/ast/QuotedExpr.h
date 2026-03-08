#pragma once

#include "Expr.h"

class QuotedExpr : public Expr {
    const Object *m_QuotedValue;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval() const override;

    QuotedExpr(const Object *quotedValue);

    static AExpr parse(CompilerContext &ctx, const Object *form);
};
