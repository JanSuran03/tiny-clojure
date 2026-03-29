#include "LocalVarExpr.h"

llvm::Value *LocalVarExpr::loadValue(CodegenContext &ctx) const {
    llvm::AllocaInst *mem = ctx.m_VariableMap.at(m_Name);
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), mem, m_Name + "_val");
}

LocalVarExpr::LocalVarExpr(std::string name,
                           unsigned functionDepth,
                           unsigned localIndex)
        : LocalBindingExpr(std::move(name)),
          m_FunctionDepth(functionDepth),
          m_LocalIndex(localIndex) {}

AExpr LocalVarExpr::clone() const {
    return std::make_unique<LocalVarExpr>(m_Name, m_FunctionDepth, m_LocalIndex);
}
