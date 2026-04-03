#include "NilExpr.h"

EmitResult NilExpr::emitIR(CodegenContext &ctx) const {
    return llvm::ConstantPointerNull::get(ctx.pointerType());
}

const Object *NilExpr::eval() const {
    return nullptr;
}
