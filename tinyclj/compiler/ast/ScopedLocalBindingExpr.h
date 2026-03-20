#pragma once

#include "LocalBindingExpr.h"

// todo: wtf is this class name
class ScopedLocalBindingExpr : public LocalBindingExpr {
public:
    llvm::Value *loadValue(CompilerContext &ctx) const override;

    ScopedLocalBindingExpr(std::string value);
};
