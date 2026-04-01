#pragma once

#include "BindingExpr.h"

class CapturedLocalExpr : public BindingExpr {
    unsigned m_ClosureEnvIndex;
    // for storing the original binding expression into env[m_ClosureEnvIndex] at runtime when the variable is captured
    std::shared_ptr<BindingExpr> origin;
public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    CapturedLocalExpr(std::string value,
                      unsigned closureEnvIndex,
                      std::shared_ptr<BindingExpr> origin);

    AExpr clone() const override;

    unsigned getClosureEnvIndex() const;

    const std::shared_ptr<BindingExpr> &getOrigin() const;
};
