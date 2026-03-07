#pragma once

#include "ConstantExpr.h"

class IntegerExpr : public ConstantExpr {
    int64_t m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    IntegerExpr(int64_t value);
};
