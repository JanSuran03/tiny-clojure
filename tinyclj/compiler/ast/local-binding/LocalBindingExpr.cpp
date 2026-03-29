#include "LocalBindingExpr.h"
#include "runtime/Runtime.h"

void LocalBindingExpr::emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const {
    using namespace llvm;
    if (dst) {
        Value *value = loadValue(ctx);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

LocalBindingExpr::LocalBindingExpr(std::string name) : m_Name(std::move(name)) {}

const std::string &LocalBindingExpr::name() const {
    return m_Name;
}
