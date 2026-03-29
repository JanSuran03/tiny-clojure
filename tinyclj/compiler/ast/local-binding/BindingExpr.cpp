#include "BindingExpr.h"
#include "runtime/Runtime.h"

void BindingExpr::emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const {
    using namespace llvm;
    if (dst) {
        Value *value = loadValue(ctx);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

BindingExpr::BindingExpr(std::string name) : m_Name(std::move(name)) {}

const std::string &BindingExpr::name() const {
    return m_Name;
}
