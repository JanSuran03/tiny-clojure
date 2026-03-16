#pragma once

#include "LocalBindingExpr.h"

class CapturedBindingExpr : public LocalBindingExpr {
    int m_ClosureEnvIndex;
public:
    llvm::Value * loadValue(CompilerContext &ctx) const override;

    CapturedBindingExpr(std::string value, int closureEnvIndex);
};
