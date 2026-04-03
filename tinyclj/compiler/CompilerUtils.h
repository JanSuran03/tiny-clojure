#pragma once

#include <vector>

#include "compiler/ast/Expr.h"

namespace CompilerUtils {
    EmitResult emitBody(const std::vector<AExpr> &body,
                        const std::string &bodyPrefix,
                        CodegenContext &ctx);

    /** Emits a constant pointer to the given object as an LLVM value. */
    llvm::Value *emitObjectPtr(const Object *obj, CodegenContext &ctx);

    /// error_message_ptr is a raw string pointer that needs to be wrapped in a TCString object
    /// before being passed to tinyclj_rt_error
    void emitThrow(llvm::Value *error_message_ptr, CodegenContext &ctx);
}
