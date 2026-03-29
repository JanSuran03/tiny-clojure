#include "CapturedLocalExpr.h"
#include "runtime/Runtime.h"

llvm::Value *CapturedLocalExpr::loadValue(CodegenContext &ctx) const {
    llvm::Value *slot = ctx.m_IRBuilder.CreateGEP(ctx.pointerArrayType(),
                                                  ctx.m_ClosureEnv,
                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx.m_LLVMContext),
                                                                         llvm::APInt(32, m_ClosureEnvIndex)),
                                                  m_Name + "_env_slot");
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), slot, m_Name + "_captured_val");
}

CapturedLocalExpr::CapturedLocalExpr(std::string value,
                                     unsigned closureEnvIndex,
                                     std::shared_ptr<BindingExpr> origin)
        : BindingExpr(std::move(value)),
          m_ClosureEnvIndex(closureEnvIndex),
          origin(std::move(origin)) {}

AExpr CapturedLocalExpr::clone() const {
    return std::make_unique<CapturedLocalExpr>(m_Name, m_ClosureEnvIndex, origin);
}

unsigned int CapturedLocalExpr::getClosureEnvIndex() const {
    return m_ClosureEnvIndex;
}

const std::shared_ptr<BindingExpr> &CapturedLocalExpr::getOrigin() const {
    return origin;
}
