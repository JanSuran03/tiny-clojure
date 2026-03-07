#include "ConstantExpr.h"

void ConstantExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    switch (mode) {
        case ExpressionMode::STATEMENT:
            break;
        case ExpressionMode::EXPRESSION: {
            llvm::Value *value = getConstantValue(ctx);
            ctx.m_IRBuilder.CreateStore(value, dst);
            break;
        }
        case ExpressionMode::RETURN: {
            llvm::Value *value = getConstantValue(ctx);
            ctx.m_IRBuilder.CreateRet(value);
            break;
        }
    }
}
