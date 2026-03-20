#pragma once

#include <vector>

#include "compiler/ast/Expr.h"

namespace CompilerUtils {
    void emitBody(const std::vector<AExpr> &body,
                  const std::string &bodyPrefix,
                  llvm::AllocaInst *dst,
                  CompilerContext &ctx);

    /** Emits a constant pointer to the given object as an LLVM value. */
    llvm::Value *emitObjectPtr(Object *obj, CompilerContext &ctx);
}
