#pragma once

#include "LocalBindingExpr.h"

class CapturedLocalExpr : public LocalBindingExpr {
    unsigned m_ClosureEnvIndex;
public:
    llvm::Value *loadValue(CodegenContext &ctx) const override;

    CapturedLocalExpr(std::string value, unsigned closureEnvIndex);

    AExpr clone() const override;

    unsigned getClosureEnvIndex() const;
};
