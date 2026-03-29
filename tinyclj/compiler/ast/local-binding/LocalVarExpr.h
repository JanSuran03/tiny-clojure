#pragma once

#include "compiler/ast/FunctionOverload.h"
#include "BindingExpr.h"

class LocalVarExpr : public BindingExpr {
    unsigned m_FunctionDepth;
    unsigned m_LocalIndex;
public:
    llvm::Value *loadValue(CodegenContext &ctx) const override;

    LocalVarExpr(std::string name,
                 unsigned functionDepth,
                 unsigned localIndex);

    AExpr clone() const override;

    unsigned getLocalIndex() const;
};
