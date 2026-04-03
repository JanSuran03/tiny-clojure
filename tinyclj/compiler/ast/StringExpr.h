#pragma once

#include "Expr.h"

class StringExpr : public Expr {
    std::string m_Value;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    StringExpr(std::string value);
};
