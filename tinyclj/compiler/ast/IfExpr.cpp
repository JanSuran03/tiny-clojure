#include "IfExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCList.h"

IfExpr::IfExpr(AExpr condExpr,
               AExpr thenExpr,
               AExpr elseExpr)
        : m_CondExpr(std::move(condExpr)),
          m_ThenExpr(std::move(thenExpr)),
          m_ElseExpr(std::move(elseExpr)) {}

EmitResult IfExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    FunctionType *get_objtype_fn_type = FunctionType::get(Type::getInt32Ty(*ctx.m_LLVMContext),
                                                          {ctx.pointerType()}, false);
    FunctionType *get_bool_value_fn_type = FunctionType::get(Type::getInt8Ty(*ctx.m_LLVMContext),
                                                             {ctx.pointerType()}, false);
    FunctionCallee get_objtype_fn = ctx.m_Module->getOrInsertFunction("tinyclj_object_get_type", get_objtype_fn_type);
    FunctionCallee get_bool_value_fn = ctx.m_Module->getOrInsertFunction("tc_boolean_valueX", get_bool_value_fn_type);

    BasicBlock *then_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.then", ctx.m_CurrentFunction);
    BasicBlock *else_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.else", ctx.m_CurrentFunction);
    BasicBlock *merge_block = BasicBlock::Create(*ctx.m_LLVMContext, "if.merge", ctx.m_CurrentFunction);
    BasicBlock *check_null_block = BasicBlock::Create(*ctx.m_LLVMContext, "check_null", ctx.m_CurrentFunction);
    BasicBlock *check_is_boolean_else_block = BasicBlock::Create(*ctx.m_LLVMContext,
                                                                 "check_bool_else", ctx.m_CurrentFunction);
    BasicBlock *check_raw_else_block = BasicBlock::Create(*ctx.m_LLVMContext, "check_else", ctx.m_CurrentFunction);

    // 1. Evaluate condition
    Value *cond_res = m_CondExpr->emitIR(ctx).value();

    // 2. If null => jump to else, otherwise check (bool)->value
    ctx.m_IRBuilder.CreateBr(check_null_block);
    ctx.m_IRBuilder.SetInsertPoint(check_null_block);
    Value *cmp_null_res = ctx.m_IRBuilder.CreateICmpEQ(cond_res, ConstantPointerNull::get(ctx.pointerType()));
    ctx.m_IRBuilder.CreateCondBr(cmp_null_res, else_block, check_is_boolean_else_block);

    // 3. Check (bool) cast, if not bool, jump to then directly (truthy)
    ctx.m_IRBuilder.SetInsertPoint(check_is_boolean_else_block);
    Value *obj_type = ctx.m_IRBuilder.CreateCall(get_objtype_fn, {cond_res}, "obj_type");
    Value *is_bool_res = ctx.m_IRBuilder.CreateICmpEQ(obj_type,
                                                      ConstantInt::get(Type::getInt32Ty(*ctx.m_LLVMContext),
                                                                       static_cast<int32_t>(ObjectType::BOOLEAN)));
    ctx.m_IRBuilder.CreateCondBr(is_bool_res, check_raw_else_block, then_block);

    // 4. Extract bool value, if false, jump to else, otherwise jump to then
    ctx.m_IRBuilder.SetInsertPoint(check_raw_else_block);
    Value *bool_value = ctx.m_IRBuilder.CreateCall(get_bool_value_fn, {cond_res}, "bool_value");
    Value *is_false = ctx.m_IRBuilder.CreateICmpEQ(bool_value, ConstantInt::get(Type::getInt8Ty(*ctx.m_LLVMContext),
                                                                                APInt(8, 0, true)));
    ctx.m_IRBuilder.CreateCondBr(is_false, else_block, then_block);

    // 5. emit then, else and jump to merge at the end
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

Object *IfExpr::eval() const {
    auto res = m_CondExpr->eval();
    if (res == nullptr ||
        (res->m_Type == ObjectType::BOOLEAN && !tc_boolean_valueX(res))) {
        return m_ElseExpr->eval();
    } else {
        return m_ThenExpr->eval();
    }
}

AExpr IfExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'if
    // TODO: Check the list length instead == 2 instead of checking after each subexpression is parsed
    if (form == nullptr) {
        throw std::runtime_error("if requires a condition expression");
    }

    auto condExpr = SemanticAnalyzer::analyze(ExpressionMode::EXPR, ctx, tc_list_first(form));
    form = tc_list_next(form);
    if (form == nullptr) {
        throw std::runtime_error("if requires a then expression");
    }
    auto thenExpr = SemanticAnalyzer::analyze(mode, ctx, tc_list_first(form));
    form = tc_list_next(form);
    AExpr elseExpr = SemanticAnalyzer::analyze(mode, ctx, tc_list_first(form));
    return std::make_unique<IfExpr>(std::move(condExpr),
                                    std::move(thenExpr),
                                    std::move(elseExpr));
}
