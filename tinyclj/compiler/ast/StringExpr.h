#pragma once

#include "ConstantExpr.h"

class StringExpr : public ConstantExpr {
    std::string m_Value;
public:
    llvm::Value * getConstantValue(CompilerContext &ctx) const override;

    StringExpr(std::string value);
};
