#pragma once

#include "LocalBindingExpr.h"

class FnArgExpr : public LocalBindingExpr {
    unsigned m_FunctionDepth;
    unsigned m_ArgIndex;
public:
    llvm::Value *loadValue(CodegenContext &ctx) const override;

    FnArgExpr(std::string value,
              unsigned functionDepth,
              unsigned localIndex);

    AExpr clone() const override;
};
