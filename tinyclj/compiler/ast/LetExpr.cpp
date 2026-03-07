#include "LetExpr.h"

void LetExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("LetExpr::emitIR not implemented");
}

LetExpr::LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, AExpr body)
        : m_Bindings(std::move(bindings)),
          m_Body(std::move(body)) {}
