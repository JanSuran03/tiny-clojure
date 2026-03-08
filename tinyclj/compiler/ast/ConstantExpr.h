#pragma once

#include "Expr.h"

class ConstantExpr : public Expr {
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const final;

    Object *eval() const final;

protected:
    virtual llvm::Value *emitConstantValue(CompilerContext &ctx) const = 0;

    virtual Object *evalConstantValue() const = 0;
};
