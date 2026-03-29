#pragma once

#include <memory>

#include "compiler/CodegenContext.h"
#include "compiler/ExpressionMode.h"
#include "types/Object.h"

struct Runtime;

struct Expr {
    virtual ~Expr() = default;

    virtual void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const = 0;

    virtual Object *eval(Runtime &runtime) const = 0;
};

using AExpr = std::unique_ptr<Expr>;
