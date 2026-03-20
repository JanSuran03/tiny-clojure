#include "compiler/CompilerUtils.h"
#include "BodyExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCList.h"

BodyExpr::BodyExpr(std::vector<AExpr> exprs)
        : m_Exprs(std::move(exprs)) {}

void BodyExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    CompilerUtils::emitBody(m_Exprs, "do", dst, ctx);
}

Object *BodyExpr::eval(Runtime &runtime) const {
    Object *result = nullptr;
    for (const auto &expr: m_Exprs) {
        result = expr->eval(runtime);
    }
    return result;
}

AExpr BodyExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'do
    std::vector<AExpr> exprs;
    for (const Object *lst = tc_list_seq(form); lst; lst = tc_list_next(lst)) {
        const Object *exprForm = tc_list_first(lst);
        bool is_last = tc_list_next(lst) == nullptr;
        exprs.push_back(SemanticAnalyzer::analyze(
                is_last ? mode : ExpressionMode::STATEMENT, // last expr in the 'do block
                ctx,
                exprForm));
    }
    return std::make_unique<BodyExpr>(std::move(exprs));
}
