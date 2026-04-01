#include "InvokeExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/rt.h"
#include "types/TCList.h"

EmitResult InvokeExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    // printf and exit for error handling in the stub
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(*ctx.m_LLVMContext),
                                                  {Type::getInt8PtrTy(*ctx.m_LLVMContext)}, true);
    FunctionCallee printf_func = ctx.m_Module->getOrInsertFunction("printf", printf_type);
    FunctionType *exit_type = FunctionType::get(Type::getVoidTy(*ctx.m_LLVMContext),
                                                {Type::getInt32Ty(*ctx.m_LLVMContext)}, false);
    FunctionCallee exit_func = ctx.m_Module->getOrInsertFunction("exit", exit_type);
    // call fn getter
    Type *sizeTy = ctx.m_IRBuilder.getIntPtrTy(ctx.m_Module->getDataLayout());
    Type *objArrayTy = PointerType::get(ctx.pointerType(), 0);
    FunctionType *callfn_getter_type = FunctionType::get(ctx.pointerType(),
                                                         {ctx.pointerType()},
                                                         false);
    FunctionCallee callfn_getter_func = ctx.m_Module->getOrInsertFunction("tinyclj_object_get_callfn",
                                                                          callfn_getter_type);

    ctx.jumpToTmpBasicBlock();
    Value *evaled_target = m_InvokeTarget->emitIR(ctx).value();

    std::vector<EmitResult> evaled_args;
    for (size_t i = 0; i < m_InvokeArgs.size(); i++) {
        const auto &arg = m_InvokeArgs[i];
        evaled_args.emplace_back(arg->emitIR(ctx));
    }

    BasicBlock *check_target_not_null = ctx.createBasicBlock("check_target_not_null");
    BasicBlock *target_is_null = ctx.createBasicBlock("target_is_null");
    BasicBlock *check_target_invokable = ctx.createBasicBlock("check_target_invokable");
    BasicBlock *target_not_invokable = ctx.createBasicBlock("target_not_invokable");
    BasicBlock *native_invoke_target = ctx.createBasicBlock("native_invoke_target");

    // (fn == null) ?
    ctx.m_IRBuilder.CreateBr(check_target_not_null);
    ctx.m_IRBuilder.SetInsertPoint(check_target_not_null);
    Value *target_is_nullptr = ctx.m_IRBuilder.CreateICmpEQ(
            evaled_target,
            ConstantPointerNull::get(ctx.pointerType()),
            "target_is_null");
    ctx.m_IRBuilder.CreateCondBr(target_is_nullptr, target_is_null, check_target_invokable);

    // fn == null => print error and exit
    ctx.m_IRBuilder.SetInsertPoint(target_is_null);
    Value *nullptr_msg = ctx.m_IRBuilder.CreateGlobalStringPtr("Error: Attempted to call `nil` at runtime.\n");
    ctx.m_IRBuilder.CreateCall(printf_func, {nullptr_msg});
    ctx.m_IRBuilder.CreateCall(exit_func, {ConstantInt::get(Type::getInt32Ty(*ctx.m_LLVMContext), 1)});
    ctx.m_IRBuilder.CreateUnreachable();

    // fn != null => check whether fn is invokable
    ctx.m_IRBuilder.SetInsertPoint(check_target_invokable);
    // native_func_ptr = evaled_target.m_CallFn
    Value *native_func_ptr = ctx.m_IRBuilder.CreateCall(callfn_getter_func, {evaled_target}, "native_func");
    Value *native_func_is_nullptr = ctx.m_IRBuilder.CreateICmpEQ(
            native_func_ptr,
            ConstantPointerNull::get(ctx.pointerType()),
            "native_func_is_null");
    ctx.m_IRBuilder.CreateCondBr(native_func_is_nullptr, target_not_invokable, native_invoke_target);

    // evaled_target.m_CallFn == null => print error and exit
    ctx.m_IRBuilder.SetInsertPoint(target_not_invokable);
    Value *not_callable_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(
            "Error: Attempted to call a non-callable object at runtime.\n");
    ctx.m_IRBuilder.CreateCall(printf_func, {not_callable_msg});
    ctx.m_IRBuilder.CreateCall(exit_func, {ConstantInt::get(Type::getInt32Ty(*ctx.m_LLVMContext), 1)});
    ctx.m_IRBuilder.CreateUnreachable();

    // invoke the native function pointer
    ctx.m_IRBuilder.SetInsertPoint(native_invoke_target);

    Value *argc_val = ConstantInt::get(sizeTy, m_InvokeArgs.size());

    AllocaInst *argv = ctx.currentInvokeArgvAllocas()[m_InvokeIndex];

    for (size_t i = 0; i < m_InvokeArgs.size(); i++) {
        Value *slot = ctx.m_IRBuilder.CreateGEP(
                ctx.pointerType(),
                argv,
                ConstantInt::get(sizeTy, i),
                "invoke_" + std::to_string(m_InvokeIndex)+ "_arg_slot_" + std::to_string(i));
        ctx.m_IRBuilder.CreateStore(evaled_args[i].value(), slot);
    }

    FunctionType *callfn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), sizeTy, objArrayTy}, // this->call(this, argc, argv)
            false);
    return ctx.m_IRBuilder.CreateCall(
            callfn_type,
            native_func_ptr,
            {evaled_target, argc_val, argv},
            "invoke_" + std::to_string(m_InvokeIndex) + "_result");
}

Object *InvokeExpr::eval() const {
    auto evaled_target = m_InvokeTarget->eval();
    std::vector<const Object *> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->eval());
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
                       std::vector<AExpr> invokeArgs,
                       size_t invokeIndex)
        : m_InvokeTarget(std::move(invokeTarget)),
          m_InvokeArgs(std::move(invokeArgs)),
          m_InvokeIndex(invokeIndex) {}

AExpr InvokeExpr::parse(ExpressionMode _, AnalyzerContext &ctx, const Object *form) {
    AExpr invokeTarget = SemanticAnalyzer::analyze(ctx, tc_list_first(form));

    std::vector<AExpr> invokeArgs;
    std::vector<LocalVarExpr> invoke_args_locals_vars;
    for (const Object *args = tc_list_next(form); args; args = tc_list_next(args)) {
        invokeArgs.emplace_back(SemanticAnalyzer::analyze(ctx, tc_list_first(args)));
    }

    unsigned invoke_index = ctx.currentInvokeArgCounts().size();
    ctx.currentInvokeArgCounts().emplace_back(invokeArgs.size());

    return std::make_unique<InvokeExpr>(std::move(invokeTarget),
                                        std::move(invokeArgs),
                                        invoke_index);
}
