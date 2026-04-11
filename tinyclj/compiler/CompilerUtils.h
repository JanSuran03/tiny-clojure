#pragma once

#include <vector>

#include "compiler/ast/Expr.h"

namespace CompilerUtils {
    EmitResult emitBody(const std::vector<AExpr> &body,
                        const std::string &bodyPrefix,
                        CodegenContext &ctx);

    /// Emits code to load the pointer to a global var with the given name, returning the pointer.
    llvm::Value *emitGlobalVar(CodegenContext &ctx, const std::string &name);

    /// error_message_ptr is a raw string pointer that needs to be wrapped in a TCString object
    /// before being passed to tinyclj_rt_error
    void emitThrow(llvm::Value *error_message_ptr, CodegenContext &ctx);
}
