#include "IfExpr.h"
#include "parser.h"
#include "types/TCBoolean.h"
#include "types/TCList.h"

IfExpr::IfExpr(AExpr condExpr, AExpr thenExpr, AExpr elseExpr)
        : m_CondExpr(std::move(condExpr)),
          m_ThenExpr(std::move(thenExpr)),
          m_ElseExpr(std::move(elseExpr)) {}

void IfExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("IfExpr::emitIR not implemented");
}

Object *IfExpr::eval() const {
    auto res = m_CondExpr->eval();
    if (res == nullptr ||
            (res->m_Type == ObjectType::BOOLEAN && !tc_boolean_valueX(res))) {
        return m_ElseExpr->eval();
    } else {
        return m_ThenExpr->eval();
    }
}

AExpr IfExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'if
    // TODO: Check the list length instead of checking one by one
    if (form == nullptr) {
        throw std::runtime_error("if requires a condition expression");
    }
    auto condExpr = Parser::analyze(mode, ctx, tc_list_first(form));
    form = tc_list_next(form);
    if (form == nullptr) {
        throw std::runtime_error("if requires a then expression");
    }
    auto thenExpr = Parser::analyze(mode, ctx, tc_list_first(form));
    form = tc_list_next(form);
    AExpr elseExpr = Parser::analyze(mode, ctx, tc_list_first(form));
    return std::make_unique<IfExpr>(std::move(condExpr), std::move(thenExpr), std::move(elseExpr));
}
