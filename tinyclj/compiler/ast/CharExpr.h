#pragma once

#include "ConstantExpr.h"
#include "../../tcdef.h"

class CharExpr : public ConstantExpr {
    char m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    CharExpr(char value);
};
