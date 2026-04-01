#pragma once

#include "compiler/ast/FunctionOverload.h"
#include "BindingExpr.h"

class LocalVarExpr : public BindingExpr {
    unsigned m_FunctionDepth;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    LocalVarExpr(std::string name,
                 unsigned functionDepth);

    AExpr clone() const override;
};
