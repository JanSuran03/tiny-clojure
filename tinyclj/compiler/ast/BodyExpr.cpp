#include "BodyExpr.h"

BodyExpr::BodyExpr(std::vector<AExpr> exprs)
        : m_Exprs(std::move(exprs)) {}

void BodyExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    for (size_t i = 0; i + 1 < m_Exprs.size(); i++) {
        m_Exprs[i]->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
    }
    m_Exprs.back()->emitIR(mode, dst, ctx);
}
