#pragma once

#include "Expr.h"

class CharExpr : public Expr {
    char m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    Object *eval() const override;

    CharExpr(char value);
};
