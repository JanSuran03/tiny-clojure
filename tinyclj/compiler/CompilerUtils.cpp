#include "CompilerUtils.h"
#include "compiler/ast/NilExpr.h"
#include "runtime/Runtime.h"

EmitResult CompilerUtils::emitBody(const std::vector<AExpr> &body,
                                   const std::string &bodyPrefix,
                                   CodegenContext &ctx) {
    if (body.empty()) {
        return NilExpr().emitIR(ctx);
    }

    EmitResult result;
    for (const auto &i: body) {
        result = i->emitIR(ctx);
    }
    return result;
}

llvm::Value *CompilerUtils::emitGlobalVar(CodegenContext &ctx, const std::string &name) {
    using namespace llvm;
    GlobalVariable *global_var = ctx.getOrCreateGlobalVariable(name);
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), global_var, "global_var_" + name);
}

void CompilerUtils::emitThrow(llvm::Value *error_message_ptr, CodegenContext &ctx) {
    // TODO: this could be optimized by emitting a call directly without the general-purpose call stub,
    //  but this is simpler for now
    using namespace llvm;
    llvm::Function *error_func = ctx.m_Module->getFunction("tinyclj_rt_error");
    if (!error_func) {
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                // parameter types: Object *self, size_t argc, const Object **argv
                {ctx.pointerType(), llvm::Type::getInt64Ty(*ctx.m_LLVMContext), ctx.pointerArrayType()},
                false // isVarArg
        );
        error_func = llvm::Function::Create(funcType,
                                            llvm::Function::ExternalLinkage,
                                            "tinyclj_rt_error",
                                            *ctx.m_Module);
    }

    llvm::Function *string_new_func = ctx.m_Module->getFunction("tc_string_new");
    if (!string_new_func) {
        llvm::FunctionType *string_new_func_type = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt8PtrTy(*ctx.m_LLVMContext)}, // parameter type: char *
                false // isVarArg
        );
        string_new_func = llvm::Function::Create(string_new_func_type,
                                                 llvm::Function::ExternalLinkage,
                                                 "tc_string_new",
                                                 *ctx.m_Module);
    }

    // Create the arguments for the function call
    Value *self_arg = ConstantPointerNull::get(ctx.pointerType()); // self is not
    Value *argc_arg = ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext), 1, false); // 1 argument
    // todo: this doesn't go with the convention that call argv's are always preallocated in the function entry block,
    // but since the function does not return AND WE CANNOT CATCH ERRORS YET, this should be fine for now - it should
    // not be possible to recursively reach this code path and grow the stack infinitely
    // however, consider refactoring this once exception can be caught in the generated code
    Value *argv_arg = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(),
                                                   ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext), 1, false),
                                                   "argv");
    Value *error_message_obj = ctx.m_IRBuilder.CreateCall(string_new_func, {error_message_ptr}, "error_message_obj");
    ctx.m_IRBuilder.CreateStore(error_message_obj, argv_arg);
    ctx.m_IRBuilder.CreateCall(error_func, {self_arg, argc_arg, argv_arg});
    // Since tinyclj_rt_error does not return, we can emit an unreachable instruction after the call
    ctx.m_IRBuilder.CreateUnreachable();
}
