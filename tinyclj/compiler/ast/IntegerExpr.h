#pragma once

#include "Expr.h"
#include "tcdef.h"

class IntegerExpr : public Expr {
    tc_int_t m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    IntegerExpr(tc_int_t value);
};
