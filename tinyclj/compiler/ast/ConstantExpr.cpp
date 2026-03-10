#include "ConstantExpr.h"

void ConstantExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (mode != ExpressionMode::STATEMENT) {
        llvm::Value *value = emitConstantValue(ctx);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

Object *ConstantExpr::eval(Runtime &runtime) const {
    return evalConstantValue();
}
