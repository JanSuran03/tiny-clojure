#include "FnArgExpr.h"

EmitResult FnArgExpr::emitIR(CodegenContext &ctx) const {
    return ctx.m_VariableMap.at(m_Name);
}

FnArgExpr::FnArgExpr(std::string value,
                     unsigned functionDepth)
        : BindingExpr(std::move(value)),
          m_FunctionDepth(functionDepth) {}

AExpr FnArgExpr::clone() const {
    return std::make_unique<FnArgExpr>(m_Name, m_FunctionDepth);
}
