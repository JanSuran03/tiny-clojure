#include "LocalBindingExpr.h"
#include "Runtime.h"

void LocalBindingExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;
    if (mode != ExpressionMode::STATEMENT) {
        Value *value;
        // Captured closure variable?
        if (this->m_ClosureEnvIndex.has_value()) {
            value = ctx.m_IRBuilder.CreateGEP(ctx.objectPointerArrayType(),
                                              ctx.m_ClosureEnv,
                                              ConstantInt::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                               APInt(32, this->m_ClosureEnvIndex.value())));
        } else {
            llvm::AllocaInst *mem = ctx.m_VariableMap.at(m_Value);
            value = ctx.m_IRBuilder.CreateLoad(ctx.objectPointerType(), mem, m_Value + "_val");
        }
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

LocalBindingExpr::LocalBindingExpr(std::string value) : m_Value(std::move(value)) {}
