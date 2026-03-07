#include "IfExpr.h"
#include "../../types/Object.h"

IfExpr::IfExpr(AExpr condExpr, AExpr thenExpr, AExpr elseExpr)
        : m_CondExpr(std::move(condExpr)),
          m_ThenExpr(std::move(thenExpr)),
          m_ElseExpr(std::move(elseExpr)) {}

void IfExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("IfExpr::emitIR not implemented");
}
