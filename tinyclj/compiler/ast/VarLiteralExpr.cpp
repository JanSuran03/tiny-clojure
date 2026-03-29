#include "NilExpr.h"
#include "VarLiteralExpr.h"
#include "compiler/CompilerUtils.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
#include "runtime/Runtime.h"

void VarLiteralExpr::emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const {
    using namespace llvm;
    if (dst) {
        // emit the Var's pointer as a literal
        Value *var_ptr = CompilerUtils::emitObjectPtr(m_Var, ctx);
        ctx.m_IRBuilder.CreateStore(var_ptr, dst);
    }
}

Object *VarLiteralExpr::eval(Runtime &runtime) const {
    return m_Var;
}

VarLiteralExpr::VarLiteralExpr(Object *var) : m_Var(var) {}

AExpr VarLiteralExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    TCList *list = static_cast<TCList *>(form->m_Data);
    if (list->m_Length != 2) {
        throw std::runtime_error("'var form must be of the form (var name).");
    }
    const Object *name = tc_list_next(form);
    name = tc_list_first(name);
    if (name == nullptr || name->m_Type != ObjectType::SYMBOL) {
        throw std::runtime_error("'var form must take a symbol as an argument.");
    }
    const std::string &var_name = static_cast<TCSymbol *>(name->m_Data)->m_Name;
    if (Object *var = Runtime::getInstance().getVar(var_name)) {
        return std::make_unique<VarLiteralExpr>(var);
    } else {
        throw std::runtime_error(std::string("Cannot resolve var: ").append(var_name).append(" in the context"));
    }
}
