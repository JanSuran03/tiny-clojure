#include "NilExpr.h"

void NilExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    switch (mode) {
        case ExpressionMode::STATEMENT:
            break;
        default:
            ctx.m_IRBuilder.CreateStore(llvm::ConstantPointerNull::get(ctx.objectPointerType()), dst);
    }
}

Object *NilExpr::eval(Runtime &runtime) const {
    return nullptr;
}
