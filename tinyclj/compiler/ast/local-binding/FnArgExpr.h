#pragma once

#include "BindingExpr.h"

class FnArgExpr : public BindingExpr {
    unsigned m_FunctionDepth;
    unsigned m_ArgIndex;
public:
    llvm::Value *loadValue(CodegenContext &ctx) const override;

    FnArgExpr(std::string value,
              unsigned functionDepth,
              unsigned localIndex);

    AExpr clone() const override;
};
