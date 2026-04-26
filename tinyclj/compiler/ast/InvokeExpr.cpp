#include "InvokeExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/rt.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"

EmitResult InvokeExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    Type *sizeTy = ctx.m_IRBuilder.getIntPtrTy(ctx.m_Module->getDataLayout());
    ctx.jumpToTmpBasicBlock();
    Value *evaled_target = m_InvokeTarget->emitIR(ctx).value();

    std::vector<EmitResult> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
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
    Value *nullptr_msg = ctx.registerGlobalString("Error: Attempted to call `nil` at runtime.\n");
    CompilerUtils::emitThrow(nullptr_msg, ctx);

    // fn != null => check whether fn is invokable
    ctx.m_IRBuilder.SetInsertPoint(check_target_invokable);
    // native_func_ptr = evaled_target.m_MethodTable->m_CallFn
    Value *native_func_ptr = Object::emitGetCallFn(ctx, evaled_target);
    Value *native_func_is_nullptr = ctx.m_IRBuilder.CreateICmpEQ(
            native_func_ptr,
            ConstantPointerNull::get(ctx.pointerType()),
            "native_func_is_null");
    ctx.m_IRBuilder.CreateCondBr(native_func_is_nullptr, target_not_invokable, native_invoke_target);

    // evaled_target.m_CallFn == null => print error and exit
    ctx.m_IRBuilder.SetInsertPoint(target_not_invokable);
    Value *not_callable_msg = ctx.registerGlobalString("Error: Attempted to call a non-callable object at runtime.\n");
    CompilerUtils::emitThrow(not_callable_msg, ctx);

    // invoke the native function pointer
    ctx.m_IRBuilder.SetInsertPoint(native_invoke_target);

    Value *argc_val = ConstantInt::get(sizeTy, m_InvokeArgs.size());

    size_t invokeIndex = Runtime::getInstance().nextId();

    // create alloca for the invoke argv in the current function frame using a builder at the function's
    // entry block
    Function *current_function = ctx.currentFunction();
    IRBuilder<> entry_builder(&current_function->getEntryBlock(), current_function->getEntryBlock().begin());
    AllocaInst *argv = entry_builder.CreateAlloca(ctx.pointerType(),
                                                  ConstantInt::get(sizeTy, m_InvokeArgs.size(), false),
                                                  "invoke_argv_alloca_" + std::to_string(invokeIndex));

    for (size_t i = 0; i < m_InvokeArgs.size(); i++) {
        Value *slot = ctx.m_IRBuilder.CreateGEP(
                ctx.pointerType(),
                argv,
                ConstantInt::get(sizeTy, i),
                "invoke_" + std::to_string(invokeIndex) + "_arg_slot_" + std::to_string(i));
        ctx.m_IRBuilder.CreateStore(evaled_args[i].value(), slot);
    }

    FunctionType *callfn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), sizeTy, ctx.pointerArrayType()}, // this->call(this, argc, argv)
            false);
    return ctx.m_IRBuilder.CreateCall(
            callfn_type,
            native_func_ptr,
            {evaled_target, argc_val, argv},
            "invoke_" + std::to_string(invokeIndex) + "_result");
}

const Object *InvokeExpr::eval() const {
    auto evaled_target = m_InvokeTarget->eval();
    std::vector<const Object *> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->eval());
    }
    if (evaled_target == nullptr) {
        throw std::runtime_error("Cannot call nil.");
    }
    CallFn callFn = evaled_target->m_MethodTable->m_CallFn;
    if (callFn == nullptr) {
        throw std::runtime_error("Object is not callable.");
    }
    return callFn(evaled_target, evaled_args.size(), evaled_args.data());
}

InvokeExpr::InvokeExpr(AExpr invokeTarget,
                       std::vector<AExpr> invokeArgs)
        : m_InvokeTarget(std::move(invokeTarget)),
          m_InvokeArgs(std::move(invokeArgs)) {}

AExpr InvokeExpr::parse(ExpressionMode _, AnalyzerContext &ctx, const Object *form) {
    AExpr invokeTarget = SemanticAnalyzer::analyze(ctx, tc_list_first(form));

    std::vector<AExpr> invokeArgs;
    std::vector<LocalVarExpr> invoke_args_locals_vars;
    for (const Object *args = tc_list_next(form); args; args = tc_list_next(args)) {
        invokeArgs.emplace_back(SemanticAnalyzer::analyze(ctx, tc_list_first(args)));
    }

    return std::make_unique<InvokeExpr>(std::move(invokeTarget),
                                        std::move(invokeArgs));
}
