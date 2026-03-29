#pragma once

#include "ConstantExpr.h"
#include "tcdef.h"

class DoubleExpr : public ConstantExpr {
    tc_double_t m_Value;
public:
    llvm::Value * emitConstantValue(CodegenContext &ctx) const override;

    Object * evalConstantValue() const override;

    DoubleExpr(tc_double_t value);
};
