#include "FnArgExpr.h"

llvm::Value *FnArgExpr::loadValue(CodegenContext &ctx) const {
    //llvm::Function *function = ctx.m_CurrentFunction;
    //llvm::Argument *arg = function->getArg(m_ArgIndex);
    //return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), arg, m_Name + "_arg_val");

    // todo: could be loaded directly if the function doesn't contain a recur expression to itself

    llvm::AllocaInst *mem = ctx.m_VariableMap.at(m_Name);
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), mem, m_Name + "_val");
}

FnArgExpr::FnArgExpr(std::string value,
                     unsigned functionDepth,
                     unsigned localIndex)
        : BindingExpr(std::move(value)),
          m_FunctionDepth(functionDepth),
          m_ArgIndex(localIndex) {}

AExpr FnArgExpr::clone() const {
    return std::make_unique<FnArgExpr>(m_Name, m_FunctionDepth, m_ArgIndex);
}
