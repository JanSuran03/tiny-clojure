#include "NilExpr.h"
#include "VarDerefExpr.h"
#include "compiler/CompilerUtils.h"
#include "types/TCVar.h"

EmitResult VarDerefExpr::emitIR(CodegenContext &ctx) const {
    llvm::Value *llvm_var_ptr = CompilerUtils::emitObjectPtr(m_Var, ctx);
    return TCVar::emitGetRoot(ctx, llvm_var_ptr);
}

const Object *VarDerefExpr::eval() const {
    return tc_var_get_root(m_Var);
}

VarDerefExpr::VarDerefExpr(Object *var) : m_Var(var) {}
