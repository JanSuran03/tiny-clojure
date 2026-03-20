#include "FunctionOverload.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionOverload::FunctionOverload(std::vector<std::string> args,
                                   bool isVariadic,
                                   bool usesClosureEnv,
                                   std::vector<AExpr> body)
        : m_Args(std::move(args)),
          m_IsVariadic(isVariadic),
          m_UsesClosureEnv(usesClosureEnv),
          m_Body(std::move(body)) {}

llvm::Function *FunctionOverload::compile(CompilerContext &ctx, const Captures &captures) const {
    std::vector<llvm::Type *> argTypes(m_Args.size(), ctx.pointerType()); // positional arguments
    if (m_UsesClosureEnv) {
        argTypes.push_back(ctx.pointerType()); // closure environment
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(ctx.pointerType(), argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType,
                                                      llvm::Function::ExternalLinkage,
                                                      "func_overload_" + std::to_string(ctx.nextId()),
                                                      ctx.m_Module);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(ctx.m_LLVMContext, "entry", function);

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

    if (m_UsesClosureEnv) {
        llvm::Argument *closure_env_arg = function->getArg(m_Args.size());
        ctx.m_ClosureEnv = closure_env_arg;
    }

    // Create a recursion point
    llvm::BasicBlock *recursion_point = ctx.createBasicBlock("recursion_point");
    ctx.m_IRBuilder.CreateBr(recursion_point);
    ctx.m_IRBuilder.SetInsertPoint(recursion_point);
    ctx.m_LoopLabels.emplace_back(recursion_point, std::move(loop_variable_storages));

    CompilerUtils::emitBody(m_Body, "fn", retAlloca, ctx);

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

    if (m_UsesClosureEnv) {
        ctx.m_ClosureEnv = prev_closure_env;
    }

    ctx.m_CurrentFunction = prev_function;

    return function;
}

FunctionOverload FunctionOverload::parse(CompilerContext &ctx, const Object *form) {
    // all overloads share the same set of captured variables - all overloads modify the same one
    // however, each overloads creates its own set of local bindings (i.e. stack frame)
    ctx.m_StackFrameBindings.emplace_back();
    ctx.m_IsClosureStack.push_back(false);
    auto &currentFrame = ctx.m_StackFrameBindings.back();
    std::vector<std::string> new_scope_vars;

    const Object *arglist = tc_list_first(form);
    form = tc_list_next(form);

    if (arglist == nullptr || arglist->m_Type != ObjectType::LIST) {
        throw std::runtime_error("fn overload requires a list of arguments");
    }

    std::vector<std::string> args;
    enum class VA_State {
        NONE,
        EXPECTING_VARARG_NAME,
        FOUND_VARARG
    };
    VA_State varargs_state = VA_State::NONE;
    for (arglist = tc_list_seq(arglist); arglist; arglist = tc_list_next(arglist)) {
        const Object *arg = tc_list_first(arglist);
        if (tinyclj_object_get_type(arg) != ObjectType::SYMBOL) {
            throw std::runtime_error("fn argument must be a symbol");
        }

        std::string arg_name = tc_symbol_valueX(arg);

        if (arg_name == "&") {
            if (varargs_state != VA_State::NONE) {
                throw std::runtime_error("Invalid use of & in argument list");
            }
            varargs_state = VA_State::EXPECTING_VARARG_NAME;
            continue;
        } else if (varargs_state == VA_State::EXPECTING_VARARG_NAME) {
            varargs_state = VA_State::FOUND_VARARG;
        }

        // todo: is this really a problem?
        if (!currentFrame.insert(arg_name).second) {
            throw std::runtime_error("Duplicate argument name: " + arg_name);
        }

        args.emplace_back(arg_name);
        if (!ctx.m_LocalBindings.contains(arg_name)) {
            ctx.m_LocalBindings.insert(arg_name);
            new_scope_vars.push_back(arg_name);
        }
    }

    if (varargs_state == VA_State::EXPECTING_VARARG_NAME) {
        throw std::runtime_error("Expected vararg name after & in argument list");
    }

    size_t old_num_recur_args = ctx.m_NumRecurArgs;
    ctx.m_NumRecurArgs = args.size();
    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        bool is_last = tc_list_next(form) == nullptr;
        const Object *expr = tc_list_first(form);
        body.emplace_back(SemanticAnalyzer::analyze(
                // discard return value of all but the last expression in the function body
                is_last ? ExpressionMode::RETURN : ExpressionMode::STATEMENT, ctx, expr));
    }

    ctx.m_NumRecurArgs = old_num_recur_args;
    for (const auto &var: new_scope_vars) {
        ctx.m_LocalBindings.erase(var);
    }
    bool is_closure = ctx.m_IsClosureStack.back();
    ctx.m_IsClosureStack.pop_back();
    ctx.m_StackFrameBindings.pop_back();

    return FunctionOverload{std::move(args),
                            varargs_state == VA_State::FOUND_VARARG,
                            is_closure,
                            std::move(body)};
}
