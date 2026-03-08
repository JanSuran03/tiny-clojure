#include "QuotedExpr.h"
#include "types/TCList.h"

void QuotedExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("Cannot emit IR for quoted expression just yet");
}

Object *QuotedExpr::eval() const {
    // todo: this hurts, rewrite this!!!
    return const_cast<Object *>(m_QuotedValue);
}

QuotedExpr::QuotedExpr(const Object *quotedValue) : m_QuotedValue(quotedValue) {}

AExpr QuotedExpr::parse(CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form);
    if (!form) {
        throw std::runtime_error("quote requires an argument");
    }
    auto quotedValue = tc_list_first(form);
    if (tc_list_next(form)) {
        throw std::runtime_error("quote takes exactly one argument");
    }
    return std::make_unique<QuotedExpr>(quotedValue);
}
