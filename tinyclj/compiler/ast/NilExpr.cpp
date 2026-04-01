#include "NilExpr.h"

EmitResult NilExpr::emitIR(CodegenContext &ctx) const {
    return llvm::ConstantPointerNull::get(ctx.pointerType());
}

Object *NilExpr::eval() const {
    return nullptr;
}
