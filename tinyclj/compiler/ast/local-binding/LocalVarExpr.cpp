#include "LocalVarExpr.h"

EmitResult LocalVarExpr::emitIR(CodegenContext &ctx) const {
    return ctx.m_VariableMap.at(m_Name);
}

LocalVarExpr::LocalVarExpr(std::string name,
                           unsigned functionDepth)
        : BindingExpr(std::move(name)),
          m_FunctionDepth(functionDepth) {}

AExpr LocalVarExpr::clone() const {
    return std::make_unique<LocalVarExpr>(m_Name, m_FunctionDepth);
}
