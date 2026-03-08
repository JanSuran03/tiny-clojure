#pragma once

#include "ConstantExpr.h"
#include "../../tcdef.h"

class DoubleExpr : public ConstantExpr {
    tc_double_t m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    DoubleExpr(tc_double_t value);
};
