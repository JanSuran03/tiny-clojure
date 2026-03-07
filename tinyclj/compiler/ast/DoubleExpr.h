#pragma once

#include "ConstantExpr.h"

class DoubleExpr : public ConstantExpr {
    double m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    DoubleExpr(double value);
};
