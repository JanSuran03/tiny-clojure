#include "QuotedExpr.h"
#include "compiler/CompilerUtils.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"

void QuotedExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (dst) {
        llvm::Value *llvm_val = CompilerUtils::emitObjectPtr(const_cast<Object *>(m_QuotedValue), ctx);
        ctx.m_IRBuilder.CreateStore(llvm_val, dst);
    }
}

Object *QuotedExpr::eval(Runtime &runtime) const {
    // todo: this hurts, rewrite this!!!
    return const_cast<Object *>(m_QuotedValue);
}

QuotedExpr::QuotedExpr(const Object *quotedValue) : m_QuotedValue(quotedValue) {
    Runtime::getInstance().registerConstant(quotedValue);
}

AExpr QuotedExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
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
