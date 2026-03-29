#pragma once

#include "compiler/ast/UnevaluatableExpr.h"

class BindingExpr : public UnevaluatableExpr {
protected:
    std::string m_Name;
public:
    virtual llvm::Value *loadValue(CodegenContext &ctx) const = 0;

    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const final;

    BindingExpr(std::string name);

    const std::string &name() const;

    virtual AExpr clone() const = 0;
};
