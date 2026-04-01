#include "RecurExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCInteger.h"
#include "types/TCList.h"

EmitResult RecurExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    std::vector<EmitResult> evaled_recur_args;
    for (const auto &arg: m_RecurArgs) {
        evaled_recur_args.emplace_back(arg->emitIR(ctx));
    }

    auto &current_loop = ctx.m_LoopLabels.back();
    BasicBlock *current_block = ctx.m_IRBuilder.GetInsertBlock();

    for (size_t i = 0; i < evaled_recur_args.size(); i++) {
        const EmitResult &arg_res = evaled_recur_args[i];
        PHINode *phi_node = current_loop.m_PhiNodes[i];
        phi_node->addIncoming(arg_res.value(), current_block);
    }

    ctx.m_IRBuilder.CreateBr(current_loop.m_Label);
    // The recur expression itself does not produce a value (if transfers control-flow instead),
    // so returning std::nullopt is appropriate here
    return std::nullopt;
}

RecurExpr::RecurExpr(std::vector<AExpr> recurArgs) : m_RecurArgs(std::move(recurArgs)) {}

AExpr RecurExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    if (mode != ExpressionMode::TAIL) {
        throw std::runtime_error("recur can only be used in tail position");
    }
    size_t currentRecurArgCount = ctx.currentRecurArgCount();
    std::vector<AExpr> recurArgs;
    form = tc_list_next(form); // skip 'recur symbol
    tc_int_t len = static_cast<TCInteger *>(tc_list_length(form)->m_Data)->m_Value;
    if (len != currentRecurArgCount) {
        throw std::runtime_error("recur arguments count mismatch - expected "
                                 + std::to_string(currentRecurArgCount) + " but got " + std::to_string(len));
    }

    while (form) {
        const Object *arg = tc_list_first(form);
        recurArgs.push_back(SemanticAnalyzer::analyze(ExpressionMode::EXPR, ctx, arg));
        form = tc_list_next(form);
    }
    return std::make_unique<RecurExpr>(std::move(recurArgs));
}
