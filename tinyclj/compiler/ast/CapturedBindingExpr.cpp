#include "CapturedBindingExpr.h"
#include "runtime/Runtime.h"

llvm::Value *CapturedBindingExpr::loadValue(CompilerContext &ctx) const {
    llvm::Value *slot = ctx.m_IRBuilder.CreateGEP(ctx.pointerArrayType(),
                                                  ctx.m_ClosureEnv,
                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext),
                                                                         llvm::APInt(32, m_ClosureEnvIndex)),
                                                  m_Value + "_env_slot");
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), slot, m_Value + "_captured_val");
}

CapturedBindingExpr::CapturedBindingExpr(std::string value,
                                         int closureEnvIndex)
        : LocalBindingExpr(std::move(value)),
          m_ClosureEnvIndex(closureEnvIndex) {}
