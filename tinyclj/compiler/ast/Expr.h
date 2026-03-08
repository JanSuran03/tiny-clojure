#pragma once

#include <memory>

#include "../CompilerContext.h"
#include "../ExpressionMode.h"
#include "../../types/Object.h"

struct Expr {
    virtual ~Expr() = default;

    virtual void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const = 0;

    virtual Object *eval() const = 0;
};

using AExpr = std::unique_ptr<Expr>;
