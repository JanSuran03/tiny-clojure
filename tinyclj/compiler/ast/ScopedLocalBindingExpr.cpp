#include "ScopedLocalBindingExpr.h"

llvm::Value *ScopedLocalBindingExpr::loadValue(CompilerContext &ctx) const {
    llvm::AllocaInst *mem = ctx.m_VariableMap.at(m_Value);
    return ctx.m_IRBuilder.CreateLoad(ctx.objectPointerType(), mem, m_Value + "_val");
}

ScopedLocalBindingExpr::ScopedLocalBindingExpr(std::string value) : LocalBindingExpr(std::move(value)) {}
