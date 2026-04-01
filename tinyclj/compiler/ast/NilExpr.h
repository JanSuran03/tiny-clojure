#pragma once

#include "Expr.h"

class NilExpr : public Expr {
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    NilExpr() = default;
};
