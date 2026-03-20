#include "LocalBindingExpr.h"
#include "Runtime.h"

void LocalBindingExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;
    if (dst) {
        Value *value = loadValue(ctx);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

LocalBindingExpr::LocalBindingExpr(std::string value) : m_Value(std::move(value)) {}
