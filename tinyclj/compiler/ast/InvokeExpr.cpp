#include "InvokeExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/rt.h"
#include "types/TCList.h"

void InvokeExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;

    // printf and exit for error handling in the thunk
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                  {Type::getInt8PtrTy(ctx.m_LLVMContext)}, true);
    FunctionCallee printf_func = ctx.m_Module.getOrInsertFunction("printf", printf_type);
    FunctionType *exit_type = FunctionType::get(Type::getVoidTy(ctx.m_LLVMContext),
                                                {Type::getInt32Ty(ctx.m_LLVMContext)}, false);
    FunctionCallee exit_func = ctx.m_Module.getOrInsertFunction("exit", exit_type);
    // call fn getter
    Type *sizeTy = ctx.m_IRBuilder.getIntPtrTy(ctx.m_Module.getDataLayout());
    Type *objArrayTy = PointerType::get(ctx.pointerType(), 0);
    FunctionType *callfn_getter_type = FunctionType::get(ctx.pointerType(),
                                                         {ctx.pointerType()},
                                                         false);
    FunctionCallee callfn_getter_func = ctx.m_Module.getOrInsertFunction("tinyclj_object_get_callfn",
                                                                         callfn_getter_type);

    llvm::AllocaInst *target_alloca = ctx.m_IRBuilder.CreateAlloca(
            ctx.pointerType(),
            nullptr,
            "evaled_target");
    ctx.jumpToTmpBasicBlock();
    m_InvokeTarget->emitIR(ExpressionMode::EXPRESSION, target_alloca, ctx);

    std::vector<llvm::AllocaInst *> arg_allocas;
    for (const auto &arg: m_InvokeArgs) {
        auto arg_alloca = ctx.m_IRBuilder.CreateAlloca(
                ctx.pointerType(),
                nullptr,
                std::string("evaled_arg_").append(std::to_string(&arg - &*m_InvokeArgs.begin())));
        arg_allocas.emplace_back(arg_alloca);
        ctx.jumpToTmpBasicBlock();
        arg->emitIR(ExpressionMode::EXPRESSION, arg_alloca, ctx);
    }

    llvm::BasicBlock *check_target_not_null = ctx.createBasicBlock("check_target_not_null");
    llvm::BasicBlock *target_is_null = ctx.createBasicBlock("target_is_null");

    llvm::BasicBlock *native_invoke_target = ctx.createBasicBlock("native_invoke_target");

    // (fn == null) ?
    ctx.m_IRBuilder.CreateBr(check_target_not_null);
    ctx.m_IRBuilder.SetInsertPoint(check_target_not_null);
    llvm::Value *target_val = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), target_alloca, "target_val");
    Value *target_is_nullptr = ctx.m_IRBuilder.CreateICmpEQ(
            target_val,
            ConstantPointerNull::get(ctx.pointerType()),
            "target_is_null");
    ctx.m_IRBuilder.CreateCondBr(target_is_nullptr, target_is_null, native_invoke_target);

    // fn == null => print error and exit
    ctx.m_IRBuilder.SetInsertPoint(target_is_null);
    Value *nullptr_msg = ctx.m_IRBuilder.CreateGlobalStringPtr("Error: Attempted to call `nil` at runtime.\n");
    ctx.m_IRBuilder.CreateCall(printf_func, {nullptr_msg});
    ctx.m_IRBuilder.CreateCall(exit_func, {ConstantInt::get(Type::getInt32Ty(ctx.m_LLVMContext), 1)});
    ctx.m_IRBuilder.CreateUnreachable();

    // fn != null => invoke native target
    ctx.m_IRBuilder.SetInsertPoint(native_invoke_target);

    Value *argcnt_val = ConstantInt::get(sizeTy, m_InvokeArgs.size());
    AllocaInst *arg_array = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), argcnt_val, "arg_array");

    for (size_t i = 0; i < m_InvokeArgs.size(); i++) {
        Value *arg_llvm_val = ctx.m_IRBuilder.CreateLoad(
                ctx.pointerType(),
                arg_allocas[i],
                "arg_val_" + std::to_string(i));
        Value *slot = ctx.m_IRBuilder.CreateGEP(
                ctx.pointerType(),
                arg_array,
                ctx.m_IRBuilder.getInt64(i),
                "arg_slot_" + std::to_string(i));
        ctx.m_IRBuilder.CreateStore(arg_llvm_val, slot);
    }

    // native_func_ptr = target.m_CallFn
    Value *native_func_ptr = ctx.m_IRBuilder.CreateCall(callfn_getter_func, {target_val}, "native_func");

    FunctionType *callfn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), sizeTy, objArrayTy}, // this->call(this, argc, argv)
            false);
    // Object *res = native_func_ptr(arg_array, argcnt)
    Value *native_call_result = ctx.m_IRBuilder.CreateCall(
            callfn_type,
            native_func_ptr,
            {target_val, argcnt_val, arg_array}, // Object *self, size_t argc, Object **argv
            "native_call_result");
    if (dst != nullptr) {
        ctx.m_IRBuilder.CreateStore(native_call_result, dst);
    }
}

Object *InvokeExpr::eval(Runtime &runtime) const {
    auto evaled_target = m_InvokeTarget->eval(runtime);
    std::vector<const Object *> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->eval(runtime));
    }
    if (evaled_target == nullptr) {
        throw std::runtime_error("Cannot call nil.");
    }
    if (evaled_target->m_Call == nullptr) {
        throw std::runtime_error("Object is not callable.");
    }
    return evaled_target->m_Call(evaled_target, evaled_args.size(), evaled_args.data());
}

InvokeExpr::InvokeExpr(AExpr invokeTarget,
                       std::vector<AExpr> invokeArgs)
        : m_InvokeTarget(std::move(invokeTarget)),
          m_InvokeArgs(std::move(invokeArgs)) {}

AExpr InvokeExpr::parse(ExpressionMode _, CompilerContext &ctx, const Object *form) {
    AExpr invokeTarget = SemanticAnalyzer::analyze(ctx, tc_list_first(form));
    std::vector<AExpr> invokeArgs;
    for (const Object *args = tc_list_next(form); args; args = tc_list_next(args)) {
        invokeArgs.emplace_back(SemanticAnalyzer::analyze(ctx, tc_list_first(args)));
    }
    return std::make_unique<InvokeExpr>(std::move(invokeTarget), std::move(invokeArgs));
}
