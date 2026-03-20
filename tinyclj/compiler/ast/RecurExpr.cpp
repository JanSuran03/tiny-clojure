#include "RecurExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCInteger.h"
#include "types/TCList.h"

void RecurExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    const LoopBase &currentLoop = ctx.m_LoopLabels.back();
    // store recur args into loop variable storages and jump to the loop label
    for (size_t i = 0; i < m_RecurArgs.size(); i++) {
        const AExpr &arg = m_RecurArgs[i];
        llvm::AllocaInst *varStorage = currentLoop.variable_storages[i];
        arg->emitIR(varStorage, ctx);
    }
    ctx.m_IRBuilder.CreateBr(currentLoop.label);
}

RecurExpr::RecurExpr(std::vector<AExpr> recurArgs) : m_RecurArgs(std::move(recurArgs)) {}

AExpr RecurExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    if (mode != ExpressionMode::RETURN) {
        throw std::runtime_error("recur can only be used in tail position");
    }
    std::vector<AExpr> recurArgs;
    form = tc_list_next(form); // skip 'recur symbol
    tc_int_t len = static_cast<TCInteger *>(tc_list_length(form)->m_Data)->m_Value;
    if (len != ctx.m_NumRecurArgs) {
        throw std::runtime_error("recur arguments count mismatch - expected "
                                 + std::to_string(ctx.m_NumRecurArgs) + " but got " + std::to_string(len));
    }

    while (form) {
        const Object *arg = tc_list_first(form);
        recurArgs.push_back(SemanticAnalyzer::analyze(ExpressionMode::EXPRESSION, ctx, arg));
        form = tc_list_next(form);
    }
    return std::make_unique<RecurExpr>(std::move(recurArgs));
}
