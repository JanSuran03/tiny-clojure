#pragma once

#include "Expr.h"

class NilExpr : public Expr {
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    NilExpr() = default;
};
