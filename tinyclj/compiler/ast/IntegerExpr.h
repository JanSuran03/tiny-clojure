#pragma once

#include "ConstantExpr.h"
#include "../../tcdef.h"

class IntegerExpr : public ConstantExpr {
    tc_int_t m_Value;
public:
    llvm::Value *emitConstantValue(CompilerContext &ctx) const override;

    Object * evalConstantValue() const override;

    IntegerExpr(tc_int_t value);
};
