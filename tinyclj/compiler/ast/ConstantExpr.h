#pragma once

#include "Expr.h"

class ConstantExpr : public Expr {
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const final;

    Object *eval(Runtime &runtime) const final;

protected:
    virtual llvm::Value *emitConstantValue(CodegenContext &ctx) const = 0;

    virtual Object *evalConstantValue() const = 0;
};
