#pragma once

#include "ConstantExpr.h"

class BooleanExpr : public ConstantExpr {
    bool m_Value;
public:
    llvm::Value *emitConstantValue(CompilerContext &ctx) const override;

    Object *evalConstantValue() const override;

    BooleanExpr(bool value);
};
