#include "NilExpr.h"

void NilExpr::emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const {
    if (dst) {
        ctx.m_IRBuilder.CreateStore(llvm::ConstantPointerNull::get(ctx.pointerType()), dst);
    }
}

Object *NilExpr::eval(Runtime &runtime) const {
    return nullptr;
}
