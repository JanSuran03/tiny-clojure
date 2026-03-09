#include "BodyExpr.h"
#include "parser.h"
#include "types/TCList.h"

BodyExpr::BodyExpr(std::vector<AExpr> exprs)
        : m_Exprs(std::move(exprs)) {}

void BodyExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    for (size_t i = 0; i + 1 < m_Exprs.size(); i++) {
        m_Exprs[i]->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
    }
    m_Exprs.back()->emitIR(mode, dst, ctx);
}

Object *BodyExpr::eval() const {
    Object *result = nullptr;
    for (const auto &expr: m_Exprs) {
        result = expr->eval();
    }
    return result;
}

AExpr BodyExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    std::vector<AExpr> exprs;
    for (const Object *lst = tc_list_seq(form); lst; lst = tc_list_next(lst)) {
        const Object *exprForm = tc_list_first(lst);
        exprs.push_back(Parser::analyze(
                tc_list_seq(lst) == nullptr ? mode : ExpressionMode::STATEMENT, // last expr in the 'do block
                ctx,
                exprForm));
    }
    return std::make_unique<BodyExpr>(std::move(exprs));
}
