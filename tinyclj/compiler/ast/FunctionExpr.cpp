#include "FunctionExpr.h"
#include "parser.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionExpr::FunctionExpr(std::string name, std::vector<std::string> args, std::vector<AExpr> body)
        : m_Name(std::move(name)),
          m_Args(std::move(args)),
          m_Body(std::move(body)) {}

void compile_thunk(const std::string &thunk_name, const std::string &fn_name, llvm::Function *internal_fn,
                   CompilerContext &ctx) {
    using namespace llvm;

    // printf and exit for error handling in the thunk
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(ctx.m_LLVMContext),
                                                  {Type::getInt8PtrTy(ctx.m_LLVMContext)}, true);
    FunctionCallee printf_func = ctx.m_Module.getOrInsertFunction("printf", printf_type);
    FunctionType *exit_type = FunctionType::get(Type::getVoidTy(ctx.m_LLVMContext),
                                                {Type::getInt32Ty(ctx.m_LLVMContext)}, false);
    FunctionCallee exit_func = ctx.m_Module.getOrInsertFunction("exit", exit_type);

    auto arg_type = ctx.objectPointerType();
    auto return_type = ctx.objectPointerType();
    FunctionType *thunk_type = llvm::FunctionType::get(return_type, {arg_type}, false);
    Function *thunk_fn = llvm::Function::Create(thunk_type,
                                                llvm::Function::ExternalLinkage,
                                                thunk_name,
                                                ctx.m_Module);

    BasicBlock *check_arg_is_nullptr_block = BasicBlock::Create(ctx.m_LLVMContext, "check_arg_is_nullptr", thunk_fn);
    BasicBlock *arg_is_nullptr_block = BasicBlock::Create(ctx.m_LLVMContext, "arg_is_nullptr", thunk_fn);

    BasicBlock *check_arg_is_list_block = BasicBlock::Create(ctx.m_LLVMContext, "check_arg_is_list", thunk_fn);
    BasicBlock *arg_is_not_list_block = BasicBlock::Create(ctx.m_LLVMContext, "arg_isnot_list", thunk_fn);

    BasicBlock *check_argcnt_block = BasicBlock::Create(ctx.m_LLVMContext, "check_argcnt", thunk_fn);
    BasicBlock *wrong_argcnt_block = BasicBlock::Create(ctx.m_LLVMContext, "wrong_argcnt", thunk_fn);

    BasicBlock *call_internal_block = BasicBlock::Create(ctx.m_LLVMContext, "thunk_call_internal", thunk_fn);

    FunctionType *get_type_fn_type = FunctionType::get(Type::getInt32Ty(ctx.m_LLVMContext), {arg_type}, false);
    FunctionType *list_length_fn_type = FunctionType::get(Type::getInt64Ty(ctx.m_LLVMContext), {arg_type}, false);
    FunctionType *list_first_fn_type = FunctionType::get(arg_type, {arg_type}, false);
    FunctionType *list_next_fn_type = FunctionType::get(arg_type, {arg_type}, false);
    FunctionCallee get_type_fn = ctx.m_Module.getOrInsertFunction("tinyclj_object_get_type", get_type_fn_type);
    FunctionCallee list_length_fn = ctx.m_Module.getOrInsertFunction("tc_list_length", list_length_fn_type);
    FunctionCallee list_first_fn = ctx.m_Module.getOrInsertFunction("tc_list_first", list_first_fn_type);
    FunctionCallee list_next_fn = ctx.m_Module.getOrInsertFunction("tc_list_next", list_next_fn_type);

    // allocate space for the arg which is used to iterate over the arglist to unpack it
    auto arg_alloca = ctx.m_IRBuilder.CreateAlloca(arg_type, nullptr, "arglist_alloca");

    // load arg, compare arg == nullptr
    ctx.m_IRBuilder.SetInsertPoint(check_arg_is_nullptr_block);
    llvm::Argument *arg = &*thunk_fn->arg_begin();
    llvm::Value *arg_is_nullptr = ctx.m_IRBuilder.CreateICmpEQ(arg, llvm::ConstantPointerNull::get(arg_type),
                                                               "arg_is_nullptr_Val");
    ctx.m_IRBuilder.CreateCondBr(arg_is_nullptr, arg_is_nullptr_block, check_arg_is_list_block);

    // arg == nullptr -> print error and exit
    ctx.m_IRBuilder.SetInsertPoint(arg_is_nullptr_block);
    llvm::Value *nullptr_msg = ctx.m_IRBuilder.CreateGlobalStringPtr("Error: Function arguments cannot be null\n");
    ctx.m_IRBuilder.CreateCall(printf_func, {nullptr_msg});
    ctx.m_IRBuilder.CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 1)});

    // arg != nullptr -> check if it's a list
    ctx.m_IRBuilder.SetInsertPoint(check_arg_is_list_block);
    llvm::Value *arg_type_id = ctx.m_IRBuilder.CreateCall(get_type_fn, {arg}, "arg_type_id_Val");
    llvm::Value *is_list = ctx.m_IRBuilder.CreateICmpEQ(arg_type_id,
                                                        llvm::ConstantInt::get(
                                                                llvm::Type::getInt32Ty(ctx.m_LLVMContext),
                                                                static_cast<int>(ObjectType::LIST)),
                                                        "arg_is_list");
    ctx.m_IRBuilder.CreateCondBr(is_list, check_argcnt_block, arg_is_not_list_block);

    // arg is not a list -> print error and exit
    ctx.m_IRBuilder.SetInsertPoint(arg_is_not_list_block);
    llvm::Value *not_list_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(
            "Error: Function arguments must be passed as a list\n");
    ctx.m_IRBuilder.CreateCall(printf_func, {not_list_msg});
    ctx.m_IRBuilder.CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 1)});

    // check list length
    ctx.m_IRBuilder.SetInsertPoint(check_argcnt_block);
    llvm::Value *arg_list_len = ctx.m_IRBuilder.CreateCall(list_length_fn, {arg}, "arg_list_len");
    llvm::Value *expected_argcnt = llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx.m_LLVMContext),
                                                          internal_fn->arg_size());
    llvm::Value *is_argcnt_correct = ctx.m_IRBuilder.CreateICmpEQ(arg_list_len, expected_argcnt,
                                                                  "is_argcnt_correct");
    ctx.m_IRBuilder.CreateCondBr(is_argcnt_correct, call_internal_block, wrong_argcnt_block);

    // wrong argcnt -> print error and exit
    ctx.m_IRBuilder.SetInsertPoint(wrong_argcnt_block);
    llvm::Value *wrong_argcnt_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(
            "Error: Wrong number of arguments (%ld) passed to a function %s, expected %ld\n");
    llvm::Value *arg_list_len_64 = ctx.m_IRBuilder.CreateZExt(arg_list_len, llvm::Type::getInt64Ty(ctx.m_LLVMContext));
    llvm::Value *fn_name_msg = ctx.m_IRBuilder.CreateGlobalStringPtr(fn_name);
    ctx.m_IRBuilder.CreateCall(printf_func, {wrong_argcnt_msg, arg_list_len_64, fn_name_msg, expected_argcnt});
    ctx.m_IRBuilder.CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 1)});

    // call the internal function with unpacked arguments
    ctx.m_IRBuilder.SetInsertPoint(call_internal_block);
    std::vector<llvm::Value *> unpacked_args;
    for (unsigned i = 0; i < internal_fn->arg_size(); i++) {
        // emit runtime calls to tc_list_first and tc_list_next to unpack the arguments from the list
        llvm::Value *list_iter = ctx.m_IRBuilder.CreateLoad(arg_type, arg_alloca, "list_iter");
        llvm::Value *arg_i = ctx.m_IRBuilder.CreateCall(list_first_fn, {list_iter}, "arg_i");
        unpacked_args.push_back(arg_i);
        if (i < internal_fn->arg_size() - 1) {
            // list_iter = tc_list_next(list_iter)
            llvm::Value *next_iter = ctx.m_IRBuilder.CreateCall(list_next_fn, {list_iter}, "next_list_iter");
            ctx.m_IRBuilder.CreateStore(next_iter, arg_alloca);
        }
    }
    // finally, call the internal function with the unpacked arguments and return its result
    llvm::Value *result = ctx.m_IRBuilder.CreateCall(internal_fn, unpacked_args, "thunk_result");
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
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "exit", function);

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

    for (const auto &expr: m_Body) {
        if (&expr == &m_Body.back()) {
            expr->emitIR(ExpressionMode::RETURN, retAlloca, ctx);
        } else {
            expr->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
        }
    }

    ctx.m_IRBuilder.CreateBr(exitBlock);
    ctx.m_IRBuilder.SetInsertPoint(exitBlock);
    ctx.m_CurrentFunction = prev_function;
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

    compile_thunk(thunk_name, m_Name, function, ctx);
}

AExpr FunctionExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'fn

    // (fn name (args...) body...)
    // or (fn (args...) body...)
    const Object *name = tc_list_first(form);

    if (name != nullptr && name->m_Type == ObjectType::SYMBOL) {
        form = tc_list_next(form); // consume the name if it's present
    } else {
        name = tc_symbol_new(("fn__" + std::to_string(ctx.m_LabelCounter++)).c_str());
    }

    const Object *arglist = tc_list_first(form);
    form = tc_list_next(form);

    if (arglist == nullptr || arglist->m_Type != ObjectType::LIST) {
        throw std::runtime_error("fn requires a list of arguments");
    }

    std::vector<std::string> args;
    std::vector<std::string> new_scope_vars;
    for (const Object *arg = tc_list_seq(arglist); arg; arg = tc_list_next(arg)) {
        if (tinyclj_object_get_type(arg) != ObjectType::SYMBOL) {
            throw std::runtime_error("fn argument must be a symbol");
        }
        args.emplace_back(tc_symbol_valueX(arg));
        if (!ctx.m_AvailableSymbols.contains(args.back())) {
            ctx.m_AvailableSymbols.insert(args.back());
            new_scope_vars.push_back(args.back());
        }
    }

    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        body.push_back(Parser::analyze(
                // discard return value of all but the last expression in the function body
                tc_list_seq(form) == nullptr ? mode : ExpressionMode::STATEMENT,
                ctx,
                tc_list_first(form)));
    }

    for (const auto &var: new_scope_vars) {
        ctx.m_AvailableSymbols.erase(var);
    }

    auto fn = std::make_unique<FunctionExpr>(
            tc_symbol_valueX(name),
            std::move(args),
            std::move(body));

    fn->compile(ctx);

    return fn;
}

void FunctionExpr::compile(CompilerContext &ctx) const {
    return emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
}

Object *FunctionExpr::eval() const {
    throw std::runtime_error("Todo: Link the generated function into the runtime using LLVM' JIT"
                             " and return a function object that wraps the generated function");
}
