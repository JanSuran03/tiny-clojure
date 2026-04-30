#include "NilExpr.h"
#include "VarLiteralExpr.h"
#include "compiler/CompilerUtils.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
#include "runtime/Runtime.h"

EmitResult VarLiteralExpr::emitIR(CodegenContext &ctx) const {
    return CompilerUtils::emitGlobalVar(ctx, static_cast<TCVar *>(m_Var->m_Data)->m_Name);
}

const Object *VarLiteralExpr::eval() const {
    return m_Var;
}

VarLiteralExpr::VarLiteralExpr(const Object *var,
                               const std::string &varName)
        : m_Var(var) {}

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
        return std::make_unique<VarLiteralExpr>(var, var_name);
    } else {
        throw std::runtime_error(std::string("Cannot resolve var: ").append(var_name).append(" in the context"));
    }
}
