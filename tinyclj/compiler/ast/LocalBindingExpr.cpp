#include "LocalBindingExpr.h"
#include "Runtime.h"

void LocalBindingExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (mode != ExpressionMode::STATEMENT) {
        llvm::AllocaInst *mem = ctx.m_VariableMap.at(m_Value);
        llvm::Value *value = ctx.m_IRBuilder.CreateLoad(ctx.objectPointerType(), mem, m_Value + "_val");
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

LocalBindingExpr::LocalBindingExpr(std::string value) : m_Value(std::move(value)) {}
