#pragma once

#include "ConstantExpr.h"

class StringExpr : public ConstantExpr {
    std::string m_Value;
public:
    llvm::Value *emitConstantValue(CodegenContext &ctx) const override;

    Object *evalConstantValue() const override;

    StringExpr(std::string value);
};
