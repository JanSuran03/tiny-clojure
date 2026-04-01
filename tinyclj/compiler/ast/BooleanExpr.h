#pragma once

#include "Expr.h"

class BooleanExpr : public Expr {
    bool m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object * eval() const override;

    BooleanExpr(bool value);
};
