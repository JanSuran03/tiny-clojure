#include "NilExpr.h"
#include "VarExpr.h"
#include "types/TCVar.h"

void VarExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;
    if (mode != ExpressionMode::STATEMENT) {
        FunctionType *get_root_fn_type = FunctionType::get(
                ctx.pointerType(),
                {ctx.pointerType()},
                false);
        FunctionCallee get_root_fn = ctx.m_Module.getOrInsertFunction("tc_var_get_root", get_root_fn_type);
        // dereference the Var's root pointer at runtime
        llvm::Value *llvm_var_ptr = ctx.m_IRBuilder.CreateIntToPtr(
                ConstantInt::get(Type::getInt64Ty(ctx.m_LLVMContext), reinterpret_cast<uint64_t>(m_Var), false),
                ctx.pointerType(),
                "var_ptr");
        Value *var_value = ctx.m_IRBuilder.CreateCall(get_root_fn, {llvm_var_ptr}, "var_value");
        ctx.m_IRBuilder.CreateStore(var_value, dst);
    }
}

VarExpr::VarExpr(TCVar *var) : m_Var(var) {}
