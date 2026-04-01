#pragma once

#include <memory>
#include <optional>

#include "compiler/CodegenContext.h"
#include "compiler/ExpressionMode.h"
#include "types/Object.h"

struct Runtime;

/**
 * In case the value is a control flow value that falls through to the next instruction, the AST node does
 * not produce a value that can be used in an expression, but instead produces a control flow change
 * (e.g. <code>recur</code> or <code>throw</code>).
 */
using EmitResult = std::optional<llvm::Value *>;



struct Expr {
    virtual ~Expr() = default;

    virtual EmitResult emitIR(CodegenContext &ctx) const = 0;

    virtual Object *eval() const = 0;
};

using AExpr = std::unique_ptr<Expr>;
