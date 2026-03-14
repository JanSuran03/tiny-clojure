#include "NilExpr.h"
#include "VarExpr.h"

void VarExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (mode != ExpressionMode::STATEMENT) {
        if (auto obj_ptr = m_Var->m_Root; obj_ptr == nullptr) {
            NilExpr().emitIR(mode, dst, ctx);
        } else {
            auto addr = llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(ctx.m_LLVMContext),
                    static_cast<uint64_t>(reinterpret_cast<uintptr_t>(obj_ptr)));
            llvm::Constant *llvm_const = llvm::ConstantExpr::getIntToPtr(addr, ctx.objectPointerType());
            ctx.m_IRBuilder.CreateStore(llvm_const, dst);
        }
    }
}

VarExpr::VarExpr(TCVar *var) : m_Var(var) {}
