#include "BooleanExpr.h"
#include "IfExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCInteger.h"
#include "types/TCList.h"

IfExpr::IfExpr(AExpr condExpr,
               AExpr thenExpr,
               AExpr elseExpr)
        : m_CondExpr(std::move(condExpr)),
          m_ThenExpr(std::move(thenExpr)),
          m_ElseExpr(std::move(elseExpr)) {}

EmitResult IfExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    BasicBlock *then_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.then", ctx.currentFunction());
    BasicBlock *else_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.else", ctx.currentFunction());
    BasicBlock *merge_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.merge", ctx.currentFunction());
    // 1. Evaluate condition
    Value *cond_res = m_CondExpr->emitIR(ctx).value();

    // 2. Compare to null or else
    Value *cmp_null_res = ctx.m_IRBuilder.CreateICmpEQ(cond_res, ConstantPointerNull::get(ctx.pointerType()));
    llvm::Value *falseBoolConstant = BooleanExpr(false).emitIR(ctx).value();
    Value *cmp_false_res = ctx.m_IRBuilder.CreateICmpEQ(cond_res, falseBoolConstant);
    Value *is_falsey_res = ctx.m_IRBuilder.CreateOr(cmp_null_res, cmp_false_res);
    ctx.m_IRBuilder.CreateCondBr(is_falsey_res, else_block, then_block);

    // 3. Emit then, else and jump to merge at the end
    ctx.m_IRBuilder.SetInsertPoint(then_block);
    auto then_res = m_ThenExpr->emitIR(ctx);
    BasicBlock *then_end = ctx.m_IRBuilder.GetInsertBlock();
    if (!ctx.currentBlockTerminated()) {
        ctx.m_IRBuilder.CreateBr(merge_block);
    }

    ctx.m_IRBuilder.SetInsertPoint(else_block);
    auto else_res = m_ElseExpr->emitIR(ctx);
    BasicBlock *else_end = ctx.m_IRBuilder.GetInsertBlock();
    if (!ctx.currentBlockTerminated()) {
        ctx.m_IRBuilder.CreateBr(merge_block);
    }

    ctx.m_IRBuilder.SetInsertPoint(merge_block);
    PHINode *phi = ctx.m_IRBuilder.CreatePHI(ctx.pointerType(), 2, "if_res");
    if (then_res.has_value()) {
        phi->addIncoming(then_res.value(), then_end);
    }
    if (else_res.has_value()) {
        phi->addIncoming(else_res.value(), else_end);
    }
    return phi;
}

const Object *IfExpr::eval() const {
    auto res = m_CondExpr->eval();
    if (res == nullptr ||
        (res->m_Type == ObjectType::BOOLEAN && !static_cast<TCBoolean *>(res->m_Data)->m_Value)) {
        return m_ElseExpr->eval();
    } else {
        return m_ThenExpr->eval();
    }
}

AExpr IfExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'if

    tc_int_t num_args = static_cast<TCInteger *>(tc_list_length(form)->m_Data)->m_Value;
    if (num_args < 2 || num_args > 3) {
        throw std::runtime_error("if requires 2 or 3 arguments");
    }

    auto condExpr = SemanticAnalyzer::analyze(ExpressionMode::EXPR, ctx, tc_list_first(form));
    form = tc_list_next(form);

    auto thenExpr = SemanticAnalyzer::analyze(mode, ctx, tc_list_first(form));
    form = tc_list_next(form);

    AExpr elseExpr = SemanticAnalyzer::analyze(mode, ctx, tc_list_first(form));
    return std::make_unique<IfExpr>(std::move(condExpr),
                                    std::move(thenExpr),
                                    std::move(elseExpr));
}
