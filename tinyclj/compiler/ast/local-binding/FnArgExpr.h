#pragma once

#include "BindingExpr.h"

class FnArgExpr : public BindingExpr {
    unsigned m_FunctionDepth;
    unsigned m_ArgIndex;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    FnArgExpr(std::string value,
              unsigned functionDepth);

    AExpr clone() const override;
};
