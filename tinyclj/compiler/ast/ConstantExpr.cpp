#include "ConstantExpr.h"

void ConstantExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (dst) {
        llvm::Value *value = emitConstantValue(ctx);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

Object *ConstantExpr::eval(Runtime &runtime) const {
    return evalConstantValue();
}
