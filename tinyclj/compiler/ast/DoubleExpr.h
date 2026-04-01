#pragma once

#include "Expr.h"
#include "tcdef.h"

class DoubleExpr : public Expr {
    tc_double_t m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    DoubleExpr(tc_double_t value);
};
