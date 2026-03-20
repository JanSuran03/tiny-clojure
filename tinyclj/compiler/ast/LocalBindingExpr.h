#pragma once

#include "UnevaluatableExpr.h"

class LocalBindingExpr : public UnevaluatableExpr {
protected:
    std::string m_Value;

    virtual llvm::Value *loadValue(CompilerContext &ctx) const = 0;

public:
    void emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const final;

    LocalBindingExpr(std::string value);
};
