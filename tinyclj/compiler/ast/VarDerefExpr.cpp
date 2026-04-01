#include "NilExpr.h"
#include "VarDerefExpr.h"
#include "compiler/CompilerUtils.h"
#include "types/TCVar.h"

EmitResult VarDerefExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;
    FunctionType *get_root_fn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType()},
            false);
    FunctionCallee get_root_fn = ctx.m_Module->getOrInsertFunction("tc_var_get_root", get_root_fn_type);
    // dereference the Var's root pointer at runtime
    llvm::Value *llvm_var_ptr = CompilerUtils::emitObjectPtr(m_Var, ctx);
    return ctx.m_IRBuilder.CreateCall(get_root_fn, {llvm_var_ptr}, "var_value");
}

Object *VarDerefExpr::eval() const {
    return const_cast<Object *>(tc_var_get_root(m_Var));
}

VarDerefExpr::VarDerefExpr(Object *var) : m_Var(var) {}
