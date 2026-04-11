#include "NilExpr.h"
#include "VarDerefExpr.h"
#include "compiler/CompilerUtils.h"
#include "types/TCVar.h"

EmitResult VarDerefExpr::emitIR(CodegenContext &ctx) const {
    // todo: instead use the var name directly lol, why use static cast
    llvm::Value *llvm_var_ptr = CompilerUtils::emitGlobalVar(ctx, static_cast<TCVar *>(m_Var->m_Data)->m_Name);
    return TCVar::emitGetRoot(ctx, llvm_var_ptr);
}

const Object *VarDerefExpr::eval() const {
    return tc_var_get_root(m_Var);
}

VarDerefExpr::VarDerefExpr(Object *var,
                           const std::string &varName,
                           AnalyzerContext &ctx)
        : m_Var(var) {
    ctx.m_ReferencedGlobalNamesStack.back().emplace(varName);
}
