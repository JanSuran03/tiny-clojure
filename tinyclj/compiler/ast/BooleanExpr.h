#pragma once

#include "Expr.h"

class BooleanExpr : public Expr {
    bool m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object * eval() const override;

    BooleanExpr(bool value);
};
