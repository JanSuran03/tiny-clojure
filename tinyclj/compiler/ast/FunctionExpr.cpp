#include "ASTUtils.h"
#include "FunctionExpr.h"

#include <utility>
#include "Runtime.h"
#include "SemanticAnalyzer.h"
#include "types/TCFunction.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionExpr::FunctionExpr(std::string name,
                           std::vector<std::string> args,
                           Captures captures,
                           std::vector<AExpr> body)
        : m_Name(std::move(name)),
          m_Args(std::move(args)),
          m_Captures(std::move(captures)),
          m_Body(std::move(body)) {}

void compile_thunk(const std::string &thunk_name,
                   const std::string &fn_name,
                   llvm::Function *internal_fn,
                   size_t expected_argcnt,
                   CompilerContext &ctx) {
    using namespace llvm;

    // printf and exit for error handling in the thunk
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                  {Type::getInt8PtrTy(ctx.m_LLVMContext)}, true);
    FunctionCallee printf_func = ctx.m_Module.getOrInsertFunction("printf", printf_type);
    FunctionType *exit_type = FunctionType::get(Type::getVoidTy(ctx.m_LLVMContext),
                                                {Type::getInt32Ty(ctx.m_LLVMContext)}, false);
    FunctionCallee exit_func = ctx.m_Module.getOrInsertFunction("exit", exit_type);

    auto arglist_type = PointerType::get(ctx.objectPointerType(), 0); // array of Object *
    auto argcnt_type = Type::getInt32Ty(ctx.m_LLVMContext);
    auto return_type = ctx.objectPointerType();
    FunctionType *thunk_type = llvm::FunctionType::get(return_type, {arglist_type, argcnt_type}, false);
    Function *thunk_fn = llvm::Function::Create(thunk_type,
                                                llvm::Function::ExternalLinkage,
                                                thunk_name,
                                                ctx.m_Module);
    BasicBlock *entry_block = BasicBlock::Create(ctx.m_LLVMContext, "entry", thunk_fn);
    BasicBlock *wrong_argcnt_block = BasicBlock::Create(ctx.m_LLVMContext, "wrong_argcnt", thunk_fn);
    BasicBlock *call_internal_block = BasicBlock::Create(ctx.m_LLVMContext, "thunk_call_internal", thunk_fn);

    ctx.m_IRBuilder.SetInsertPoint(entry_block);

    // check list length
    Value *packed_arglist = thunk_fn->getArg(0);
    Value *actual_argcnt = thunk_fn->getArg(1);
    Value *expected_argcnt_llvm_val = ConstantInt::get(ctx.m_LLVMContext, APInt(32, expected_argcnt, false));
    Value *is_argcnt_correct = ctx.m_IRBuilder.CreateICmpEQ(actual_argcnt, expected_argcnt_llvm_val);
    ctx.m_IRBuilder.CreateCondBr(is_argcnt_correct, call_internal_block, wrong_argcnt_block);

    // wrong argcnt -> print error and exit
    ctx.m_IRBuilder.SetInsertPoint(wrong_argcnt_block);
    llvm::Value *wrong_argcnt_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(
            "Error: Wrong number of arguments (%u) passed to a function %s, expected %ld\n");
    llvm::Value *fn_name_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(fn_name);
    ctx.m_IRBuilder.CreateCall(printf_func, {wrong_argcnt_msg, actual_argcnt, fn_name_msg, expected_argcnt_llvm_val});
    ctx.m_IRBuilder.CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 1)});
    ctx.m_IRBuilder.CreateUnreachable();

    // call the internal function with unpacked arguments
    ctx.m_IRBuilder.SetInsertPoint(call_internal_block);
    std::vector<llvm::Value *> direct_args;
    for (size_t i = 0; i < expected_argcnt; i++) {
        direct_args.emplace_back(ctx.m_IRBuilder.CreateGEP(ctx.objectPointerType(),
                                                           packed_arglist,
                                                           ctx.m_IRBuilder.getInt64(i)));
    }
    // finally, call the internal function with the unpacked arguments and return its result
    llvm::Value *result = ctx.m_IRBuilder.CreateCall(internal_fn, direct_args, "thunk_result");
    ctx.m_IRBuilder.CreateRet(result);
}

void FunctionExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (!m_Captures.empty()) {
        throw std::runtime_error("Closures are not supported yet");
    }

    // the function that is actually exposed to the user - takes Object * as a list of arguments.
    // checks the list length, then calls the internal function with the unpacked arguments.
    std::string thunk_name = m_Name + "__thunk";

    std::vector<llvm::Type *> argTypes(m_Args.size(), ctx.objectPointerType());
    llvm::FunctionType *funcType = llvm::FunctionType::get(ctx.objectPointerType(), argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, m_Name, ctx.m_Module);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "entry", function);
    //llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "exit", function);

    llvm::Function *prev_function = ctx.m_CurrentFunction;
    ctx.m_CurrentFunction = function;
    ctx.m_IRBuilder.SetInsertPoint(entryBlock);

    // allocate space for the return value
    llvm::AllocaInst *retAlloca = ctx.m_IRBuilder.CreateAlloca(ctx.objectPointerType(), nullptr, "retval");

    // allocate space for arguments and store them
    std::unordered_map<std::string, llvm::AllocaInst *> shadowed_allocas;
    int arg_index = 0;
    for (llvm::Argument &arg: function->args()) {
        auto &arg_name = m_Args[arg_index++];
        llvm::AllocaInst *arg_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.objectPointerType(), nullptr, arg_name);
        ctx.m_IRBuilder.CreateStore(&arg, arg_alloca);
        if (auto old_alloca = ctx.m_VariableMap.find(arg_name); old_alloca != ctx.m_VariableMap.end()) {
            shadowed_allocas[arg_name] = old_alloca->second;
        } else {
            shadowed_allocas[arg_name] = nullptr;
        }
        ctx.m_VariableMap[arg_name] = arg_alloca;
    }

    AstUtils::emitBody(m_Body, "fn", ExpressionMode::RETURN, retAlloca, ctx);

    ctx.m_CurrentFunction = prev_function;

    // the following should not be needed - the builder is already placed after the last block in the function
    // ctx.m_IRBuilder.CreateBr(exitBlock);
    // ctx.m_IRBuilder.SetInsertPoint(exitBlock);
    llvm::Value *retVal = ctx.m_IRBuilder.CreateLoad(ctx.objectPointerType(), retAlloca, "retVal");
    ctx.m_IRBuilder.CreateRet(retVal);

    // restore shadowed variables in the context
    for (const auto &[name, alloca]: shadowed_allocas) {
        if (alloca) {
            ctx.m_VariableMap[name] = alloca;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }

    compile_thunk(thunk_name, m_Name, function, m_Args.size(), ctx);
}

AExpr FunctionExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    // todo: use unique_ptr to clean up memory when throwing exceptions
    FunctionFrame *currentFrame = ctx.m_CurrentFunctionFrame = new FunctionFrame(ctx.m_CurrentFunctionFrame);
    form = tc_list_next(form); // consume 'fn

    // (fn name (args...) body...)
    // or (fn (args...) body...)
    const Object *name = tc_list_first(form);

    if (name != nullptr && name->m_Type == ObjectType::SYMBOL) {
        form = tc_list_next(form); // consume the name if it's present
    } else {
        name = tc_symbol_new(("fn__" + std::to_string(ctx.m_IdCounter++)).c_str());
    }

    const Object *arglist = tc_list_first(form);
    form = tc_list_next(form);

    if (arglist == nullptr || arglist->m_Type != ObjectType::LIST) {
        throw std::runtime_error("fn requires a list of arguments");
    }

    std::vector<std::string> args;
    std::vector<std::string> new_scope_vars;
    for (arglist = tc_list_seq(arglist); arglist; arglist = tc_list_next(arglist)) {
        const Object *arg = tc_list_first(arglist);
        if (tinyclj_object_get_type(arg) != ObjectType::SYMBOL) {
            throw std::runtime_error("fn argument must be a symbol");
        }

        std::string arg_name = tc_symbol_valueX(arg);
        currentFrame->m_Locals.emplace(arg_name);

        args.emplace_back(arg_name);
        if (!ctx.m_LocalBindings.contains(args.back())) {
            ctx.m_LocalBindings.insert(args.back());
            new_scope_vars.push_back(args.back());
        }
    }

    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        body.push_back(SemanticAnalyzer::analyze(
                // discard return value of all but the last expression in the function body
                tc_list_seq(form) == nullptr ? mode : ExpressionMode::STATEMENT,
                ctx,
                tc_list_first(form)));
    }

    for (const auto &var: new_scope_vars) {
        ctx.m_LocalBindings.erase(var);
    }

    auto fn = std::make_unique<FunctionExpr>(
            tc_symbol_valueX(name),
            std::move(args),
            currentFrame->m_Captures, // todo: std::move
            std::move(body));

    fn->compile(ctx);

    ctx.m_CurrentFunctionFrame = ctx.m_CurrentFunctionFrame->m_ParentFrame;

    return fn;
}

void FunctionExpr::compile(CompilerContext &ctx) const {
    return emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
}

bool FunctionExpr::isClosure() {
    return !m_Captures.empty();
}

Object *FunctionExpr::eval(Runtime &runtime) const {
    auto &jit = runtime.getJIT();

    const CallFn thunk_fn = reinterpret_cast<const CallFn>(
            jit->lookup(m_Name + "__thunk")->getAddress());
    return tc_function_new(thunk_fn, m_Name.c_str());
}
