#include "IfExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCList.h"

IfExpr::IfExpr(AExpr condExpr, AExpr thenExpr, AExpr elseExpr)
        : m_CondExpr(std::move(condExpr)),
          m_ThenExpr(std::move(thenExpr)),
          m_ElseExpr(std::move(elseExpr)) {}

void IfExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;

    FunctionType *get_objtype_fn_type = FunctionType::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                          {ctx.pointerType()}, false);
    FunctionType *get_bool_value_fn_type = FunctionType::get(Type::getInt8Ty(ctx.m_LLVMContext),
                                                             {ctx.pointerType()}, false);
    FunctionCallee get_objtype_fn = ctx.m_Module.getOrInsertFunction("tinyclj_object_get_type", get_objtype_fn_type);
    FunctionCallee get_bool_value_fn = ctx.m_Module.getOrInsertFunction("tc_boolean_valueX", get_bool_value_fn_type);

    BasicBlock *then_block = BasicBlock::Create(ctx.m_LLVMContext, "then", ctx.m_CurrentFunction);
    BasicBlock *else_block = BasicBlock::Create(ctx.m_LLVMContext, "else", ctx.m_CurrentFunction);
    BasicBlock *merge_block = BasicBlock::Create(ctx.m_LLVMContext, "merge", ctx.m_CurrentFunction);
    BasicBlock *check_null_block = BasicBlock::Create(ctx.m_LLVMContext, "check_null", ctx.m_CurrentFunction);
    BasicBlock *check_is_boolean_else_block = BasicBlock::Create(ctx.m_LLVMContext,
                                                                 "check_bool_else", ctx.m_CurrentFunction);
    BasicBlock *check_raw_else_block = BasicBlock::Create(ctx.m_LLVMContext, "check_else", ctx.m_CurrentFunction);

    // 1. Evaluate condition
    AllocaInst *cond_res_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType());
    m_CondExpr->emitIR(cond_res_alloca, ctx);

    // 2. If null => jump to else, otherwise check (bool)->value
    ctx.m_IRBuilder.CreateBr(check_null_block);
    ctx.m_IRBuilder.SetInsertPoint(check_null_block);
    Value *object = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), cond_res_alloca);
    Value *cmp_null_res = ctx.m_IRBuilder.CreateICmpEQ(object, ConstantPointerNull::get(ctx.pointerType()));
    ctx.m_IRBuilder.CreateCondBr(cmp_null_res, else_block, check_is_boolean_else_block);

    // 3. Check (bool) cast, if not bool, jump to then directly (truthy)
    ctx.m_IRBuilder.SetInsertPoint(check_is_boolean_else_block);
    Value *obj_type = ctx.m_IRBuilder.CreateCall(get_objtype_fn, {object}, "obj_type");
    Value *is_bool_res = ctx.m_IRBuilder.CreateICmpEQ(obj_type,
                                                      ConstantInt::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                                       static_cast<int32_t>(ObjectType::BOOLEAN)));
    ctx.m_IRBuilder.CreateCondBr(is_bool_res, check_raw_else_block, then_block);

    // 4. Extract bool value, if false, jump to else, otherwise jump to then
    ctx.m_IRBuilder.SetInsertPoint(check_raw_else_block);
    Value *bool_value = ctx.m_IRBuilder.CreateCall(get_bool_value_fn, {object}, "bool_value");
    Value *is_false = ctx.m_IRBuilder.CreateICmpEQ(bool_value, ConstantInt::get(Type::getInt8Ty(ctx.m_LLVMContext),
                                                                                APInt(8, 0, true)));
    ctx.m_IRBuilder.CreateCondBr(is_false, else_block, then_block);

    // 5. emit then, else and jump to merge at the end
    ctx.m_IRBuilder.SetInsertPoint(then_block);
    m_ThenExpr->emitIR(dst, ctx);
    if (!ctx.currentBlockTerminated()) {
        ctx.m_IRBuilder.CreateBr(merge_block);
    }

    ctx.m_IRBuilder.SetInsertPoint(else_block);
    m_ElseExpr->emitIR(dst, ctx);
    if (!ctx.currentBlockTerminated()) {
        ctx.m_IRBuilder.CreateBr(merge_block);
    }

    ctx.m_IRBuilder.SetInsertPoint(merge_block);
}

Object *IfExpr::eval(Runtime &runtime) const {
    auto res = m_CondExpr->eval(runtime);
    if (res == nullptr ||
        (res->m_Type == ObjectType::BOOLEAN && !tc_boolean_valueX(res))) {
        return m_ElseExpr->eval(runtime);
    } else {
        return m_ThenExpr->eval(runtime);
    }
}

AExpr IfExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'if
    // TODO: Check the list length instead of checking one by one
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
    return std::make_unique<IfExpr>(std::move(condExpr), std::move(thenExpr), std::move(elseExpr));
}
