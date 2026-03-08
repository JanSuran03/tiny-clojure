#pragma once

#include "ConstantExpr.h"

class CharExpr : public ConstantExpr {
    char m_Value;
public:
    llvm::Value * emitConstantValue(CompilerContext &ctx) const override;

    Object * evalConstantValue() const override;

    CharExpr(char value);
};
