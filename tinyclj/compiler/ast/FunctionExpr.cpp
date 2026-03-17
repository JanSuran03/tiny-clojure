#include "compiler/CompilerUtils.h"
#include "FunctionExpr.h"

#include <utility>
#include "Runtime.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCFunction.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
#include <iostream>

FunctionExpr::FunctionExpr(std::string name,
                           std::vector<std::string> args,
                           Captures captures,
                           std::vector<AExpr> body)
        : m_Name(std::move(name)),
          m_Args(std::move(args)),
          m_Captures(std::move(captures)),
          m_Body(std::move(body)) {}

/** The thunk function that is called at runtime and thus needs to have a consistent signature:
 * thunk_fn(Object *self, size_t argcnt, Object **argv)
 * The thunk function checks the argument count, unpacks the arguments from the array, and calls the
 * internal function with unpacked arguments
 * In case of a closure, the thunk also needs to retrieve the closure environment pointer from the self argument
 * and pass it to the internal function as its last argument.
 */
void compile_thunk(const std::string &thunk_name,
                   const std::string &fn_name,
                   llvm::Function *internal_fn,
                   bool is_closure,
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

    auto arglist_type = ctx.pointerArrayType(); // array of Object *
    auto argcnt_type = Type::getInt64Ty(ctx.m_LLVMContext);
    auto return_type = ctx.pointerType();
    // the thunk type is Object *thunk(Object *self, size_t argcnt, Object **argv)
    FunctionType *thunk_type = llvm::FunctionType::get(return_type, {ctx.pointerType(),
                                                                     argcnt_type,
                                                                     arglist_type}, false);
    Function *thunk_fn = llvm::Function::Create(thunk_type,
                                                llvm::Function::ExternalLinkage,
                                                thunk_name,
                                                ctx.m_Module);
    BasicBlock *entry_block = BasicBlock::Create(ctx.m_LLVMContext, "entry", thunk_fn);
    BasicBlock *wrong_argcnt_block = BasicBlock::Create(ctx.m_LLVMContext, "wrong_argcnt", thunk_fn);
    BasicBlock *call_internal_block = BasicBlock::Create(ctx.m_LLVMContext, "thunk_call_internal", thunk_fn);

    ctx.m_IRBuilder.SetInsertPoint(entry_block);

    // check list length
    Value *self_arg = thunk_fn->getArg(0);
    Value *actual_argcnt = thunk_fn->getArg(1);
    Value *packed_arglist = thunk_fn->getArg(2);
    Value *expected_argcnt_llvm_val = ConstantInt::get(ctx.m_LLVMContext, APInt(64, expected_argcnt, false));
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
        // the thunk receives the arguments as an array of Object *, so we need to load each argument from the array
        // before passing it to the internal function
        Value *slot = ctx.m_IRBuilder.CreateGEP(ctx.pointerType(),
                                                packed_arglist,
                                                ctx.m_IRBuilder.getInt64(i));
        Value *arg = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), slot, "arg_" + std::to_string(i));
        direct_args.push_back(arg);
    }

    if (is_closure) {
        // for closures, the thunk also needs to pass the closure environment pointer to the internal function
        FunctionType *tc_closure_get_envX_type = FunctionType::get(ctx.pointerType(),
                                                                   {ctx.pointerType()}, false);
        FunctionCallee tc_closure_get_envX_func = ctx.m_Module.getOrInsertFunction("tc_closure_get_envX",
                                                                                   tc_closure_get_envX_type);
        Value *closure_env = ctx.m_IRBuilder.CreateCall(tc_closure_get_envX_func, {self_arg}, "closure_env");
        direct_args.push_back(closure_env);
    }

    // finally, call the internal function with the unpacked arguments and return its result
    llvm::Value *result = ctx.m_IRBuilder.CreateCall(internal_fn, direct_args, "thunk_result");
    ctx.m_IRBuilder.CreateRet(result);
}

AExpr FunctionExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    // todo: use unique_ptr to clean up memory when throwing exceptions
    FunctionFrame *currentFrame = new FunctionFrame(ctx.m_CurrentFunctionFrame);
    ctx.m_CurrentFunctionFrame = currentFrame;
    form = tc_list_next(form); // consume 'fn

    // (fn name (args...) body...)
    // or (fn (args...) body...)
    const Object *name = tc_list_first(form);

    if (name != nullptr && name->m_Type == ObjectType::SYMBOL) {
        form = tc_list_next(form); // consume the name if it's present
    } else {
        name = tc_symbol_new(("fn__" + std::to_string(ctx.nextId())).c_str());
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

    size_t old_num_recur_args = ctx.m_NumRecurArgs;
    ctx.m_NumRecurArgs = args.size();
    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        bool is_last = tc_list_next(form) == nullptr;
        const Object *expr = tc_list_first(form);
        body.push_back(SemanticAnalyzer::analyze(
                // discard return value of all but the last expression in the function body
                is_last ? ExpressionMode::RETURN : ExpressionMode::STATEMENT, ctx, expr));
    }

    ctx.m_NumRecurArgs = old_num_recur_args;
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

/** This compiles the internal function that takes unpacked positional arguments (and closure environment in
 * case of a closure) and creates a thunk function that has a consistent signature and can be called at runtime:
 * thunk_fn(Object *self, size_t argcnt, Object **argv)
 */
void FunctionExpr::compile(CompilerContext &ctx) const {
    std::vector<llvm::Type *> argTypes(m_Args.size(), ctx.pointerType()); // positional arguments
    if (isClosure()) {
        argTypes.emplace_back(ctx.pointerType()); // closure environment
    }


    llvm::FunctionType *funcType = llvm::FunctionType::get(ctx.pointerType(), argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, m_Name, ctx.m_Module);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "entry", function);
    //llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "exit", function);

    llvm::Function *prev_function = ctx.m_CurrentFunction;
    ctx.m_CurrentFunction = function;
    ctx.m_IRBuilder.SetInsertPoint(entryBlock);

    // allocate space for the return value
    llvm::AllocaInst *retAlloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), nullptr, "retval");

    // allocate space for arguments and store them
    std::unordered_map<std::string, llvm::AllocaInst *> shadowed_allocas;
    std::vector<llvm::AllocaInst *> loop_variable_storages;
    int arg_index = 0;
    for (size_t i = 0; i < m_Args.size(); i++) {
        llvm::Argument &arg = function->args().begin()[i];
        auto &arg_name = m_Args[arg_index++];
        llvm::AllocaInst *arg_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), nullptr, arg_name);
        ctx.m_IRBuilder.CreateStore(&arg, arg_alloca);
        if (auto old_alloca = ctx.m_VariableMap.find(arg_name); old_alloca != ctx.m_VariableMap.end()) {
            shadowed_allocas[arg_name] = old_alloca->second;
        } else {
            shadowed_allocas[arg_name] = nullptr;
        }
        ctx.m_VariableMap[arg_name] = arg_alloca;
        loop_variable_storages.emplace_back(arg_alloca);
    }

    llvm::Argument *prev_closure_env = ctx.m_ClosureEnv;

    if (isClosure()) {
        llvm::Argument *closure_env_arg = function->getArg(m_Args.size());
        ctx.m_ClosureEnv = closure_env_arg;
    }

    // Create a recursion point
    llvm::BasicBlock *recursion_point = ctx.createBasicBlock("recursion_point");
    ctx.m_IRBuilder.CreateBr(recursion_point);
    ctx.m_IRBuilder.SetInsertPoint(recursion_point);
    ctx.m_LoopLabels.emplace_back(recursion_point, std::move(loop_variable_storages));

    CompilerUtils::emitBody(m_Body, "fn", ExpressionMode::RETURN, retAlloca, ctx);

    ctx.m_LoopLabels.pop_back();

    if (!ctx.currentBlockTerminated()) {
        llvm::Value *retVal = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), retAlloca, "retVal");
        ctx.m_IRBuilder.CreateRet(retVal);
    }

    // restore shadowed variables in the context
    for (const auto &[name, alloca]: shadowed_allocas) {
        if (alloca) {
            ctx.m_VariableMap[name] = alloca;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }

    compile_thunk(m_ThunkName, m_Name, function, isClosure(), m_Args.size(), ctx);

    if (isClosure()) {
        ctx.m_ClosureEnv = prev_closure_env;
    }

    ctx.m_CurrentFunction = prev_function;
}

void FunctionExpr::emitIR(ExpressionMode _, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;
    // For simple functions (without captures), the compiler needs to emit instructions to create a function object
    // that points to the thunk and store it into dst

    // For closures, the compiler needs to emit instructions to create an environment struct from the captured
    // variables, then create a closure object that points to the thunk and store it into dst

    llvm::Function *thunk_fn = ctx.m_Module.getFunction(m_ThunkName);
    if (!thunk_fn) {
        throw std::runtime_error("Cannot find the thunk function for " + m_Name);
    }

    // Object *thunk_fn(Object *self, size_t arg, Object **argv)
    FunctionType *call_fn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), Type::getInt64Ty(ctx.m_LLVMContext), ctx.pointerArrayType()},
            false);

    if (isClosure()) {
        // allocate the closure environment struct on the heap (the env struct is an array of Object *)
        FunctionType *allocate_env_fn_type = FunctionType::get(
                ctx.pointerType(),
                {Type::getInt64Ty(ctx.m_LLVMContext)}, false);
        FunctionCallee allocate_env_fn = ctx.m_Module.getOrInsertFunction("tc_closure_allocate_env",
                                                                          allocate_env_fn_type);
        FunctionType *closure_new_fn_type = FunctionType::get(
                ctx.pointerType(),
                {PointerType::get(call_fn_type, 0), ctx.pointerType()}, // thunk fn ptr, env struct ptr
                false);
        FunctionCallee closure_new_fn = ctx.m_Module.getOrInsertFunction("tc_closure_new", closure_new_fn_type);
        Value *env_struct_ptr = ctx.m_IRBuilder.CreateCall(
                allocate_env_fn,
                {ConstantInt::get(Type::getInt64Ty(ctx.m_LLVMContext), m_Captures.size(), false)},
                "closure_env");
        // store captured variables into the env struct
        for (const auto &[var_name, index]: m_Captures) {
            if (auto it = ctx.m_VariableMap.find(var_name); it != ctx.m_VariableMap.end()) {
                AllocaInst *captured_var_alloca = it->second;
                Value *captured_var_value = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(),
                                                                       captured_var_alloca,
                                                                       var_name + "_captured");
                Value *env_slot_ptr = ctx.m_IRBuilder.CreateGEP(
                        ctx.pointerType(),
                        env_struct_ptr,
                        ConstantInt::get(Type::getInt64Ty(ctx.m_LLVMContext), index, false),
                        var_name + "_env_slot_ptr");
                ctx.m_IRBuilder.CreateStore(captured_var_value, env_slot_ptr);
            }
        }

        // create the closure object with the thunk pointer and env struct pointer
        Value *closure_obj = ctx.m_IRBuilder.CreateCall(closure_new_fn, {thunk_fn, env_struct_ptr}, "closure_obj");
        if (dst != nullptr) {
            ctx.m_IRBuilder.CreateStore(closure_obj, dst);
        }
    } else {
        // create the function object with the thunk pointer
        FunctionType *function_new_fn_type = FunctionType::get(
                ctx.pointerType(),
                {PointerType::get(call_fn_type, 0), Type::getInt8PtrTy(ctx.m_LLVMContext)},
                false);
        FunctionCallee function_new_fn = ctx.m_Module.getOrInsertFunction("tc_function_new", function_new_fn_type);
        Constant *fn_name_const = ctx.m_IRBuilder.CreateGlobalStringPtr(m_Name, "fn_name");
        Value *function_obj = ctx.m_IRBuilder.CreateCall(function_new_fn, {thunk_fn, fn_name_const}, "function_obj");
        if (dst != nullptr) {
            ctx.m_IRBuilder.CreateStore(function_obj, dst);
        }
    }
}

bool FunctionExpr::isClosure() const {
    return !m_Captures.empty();
}

Object *FunctionExpr::eval(Runtime &runtime) const {
    auto &jit = runtime.getJIT();

    const CallFn thunk_fn = reinterpret_cast<const CallFn>(
            jit->lookup(m_Name + "__thunk")->getAddress());
    return tc_function_new(thunk_fn, m_Name.c_str());
}
