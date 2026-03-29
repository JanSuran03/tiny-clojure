#pragma once

#include "compiler/ast/UnevaluatableExpr.h"

class LocalBindingExpr : public UnevaluatableExpr {
protected:
    std::string m_Name;

    virtual llvm::Value *loadValue(CodegenContext &ctx) const = 0;

public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const final;

    LocalBindingExpr(std::string name);

    const std::string &name() const;

    virtual AExpr clone() const = 0;
};
