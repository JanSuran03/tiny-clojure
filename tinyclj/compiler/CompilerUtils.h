#pragma once

#include <vector>

#include "compiler/ast/Expr.h"

namespace CompilerUtils {
    EmitResult emitBody(const std::vector<AExpr> &body,
                        const std::string &bodyPrefix,
                        CodegenContext &ctx);

    /** Emits a constant pointer to the given object as an LLVM value. */
    llvm::Value *emitObjectPtr(Object *obj, CodegenContext &ctx);
}
