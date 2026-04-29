#include "StaticInvokeExpr.h"
#include "runtime/Runtime.h"
#include "types/TCFunction.h"

// todo: maybe share some code with DynamicInvokeExpr?
EmitResult StaticInvokeExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    Type *sizeTy = ctx.m_IRBuilder.getIntPtrTy(ctx.m_Module->getDataLayout());
    ctx.jumpToTmpBasicBlock();

    std::vector<EmitResult> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->emitIR(ctx));
    }

    size_t invokeIndex = Runtime::nextId();

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

    std::string function_name = static_cast<TCFunction *>(m_NativeFunctionObject->m_Data)->m_Name;
    std::string call_stub_name;
    if (function_name.starts_with("tinyclj_rt")) {
        call_stub_name = function_name;
    } else {
        call_stub_name = function_name + "__stub";
    }
    Function *externalNativeFunc = ctx.m_Module->getFunction(call_stub_name);
    if (!externalNativeFunc) {
        externalNativeFunc = llvm::Function::Create(callfn_type,
                                                    llvm::Function::ExternalLinkage,
                                                    call_stub_name,
                                                    *ctx.m_Module);
    }
    Value *native_func_ptr = ctx.m_IRBuilder.CreateBitCast(externalNativeFunc,
                                                           callfn_type->getPointerTo(),
                                                           "native_func_ptr");
    // no need to pass a valid `this` pointer since static invoke doesn't rely on any instance data. Pass null instead.
    Value *self_ptr = ConstantPointerNull::get(ctx.pointerType());
    Value *argc_val = ConstantInt::get(sizeTy, m_InvokeArgs.size());

    return ctx.m_IRBuilder.CreateCall(
            callfn_type,
            native_func_ptr,
            {self_ptr, argc_val, argv},
            "invoke_" + std::to_string(invokeIndex) + "_result");

}

const Object *StaticInvokeExpr::eval() const {
    std::vector<const Object *> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->eval());
    }
    return m_NativeFunctionObject->m_MethodTable->m_CallFn(nullptr, evaled_args.size(), evaled_args.data());
}

StaticInvokeExpr::StaticInvokeExpr(const Object *nativeFunctionObject, std::vector<AExpr> invokeArgs)
        : m_NativeFunctionObject(nativeFunctionObject),
          m_InvokeArgs(std::move(invokeArgs)) {}
