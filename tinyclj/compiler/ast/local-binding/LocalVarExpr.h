#pragma once

#include "compiler/ast/FunctionOverload.h"
#include "LocalBindingExpr.h"

class LocalVarExpr : public LocalBindingExpr {
    unsigned m_FunctionDepth;
    unsigned m_LocalIndex;
public:
    llvm::Value *loadValue(CodegenContext &ctx) const override;

    LocalVarExpr(std::string name,
                 unsigned functionDepth,
                 unsigned localIndex);

    AExpr clone() const override;
};
