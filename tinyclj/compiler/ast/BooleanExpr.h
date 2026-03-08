#pragma once

#include "ConstantExpr.h"

class BooleanExpr : public ConstantExpr {
    bool m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    BooleanExpr(bool value);
};
