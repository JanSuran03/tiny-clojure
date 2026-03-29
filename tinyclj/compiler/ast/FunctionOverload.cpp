#include "FunctionOverload.h"
#include "local-binding/CapturedLocalExpr.h"
#include "local-binding/FnArgExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionOverload::FunctionOverload(std::vector<FnArgExpr> args,
                                   bool isVariadic,
                                   bool usesClosureEnv,
                                   std::vector<AExpr> body)
        : m_Args(std::move(args)),
          m_IsVariadic(isVariadic),
          m_UsesClosureEnv(usesClosureEnv),
          m_Body(std::move(body)) {}

llvm::Function *FunctionOverload::compile(CodegenContext &ctx, const Captures &captures) const {
    std::vector<llvm::Type *> argTypes(m_Args.size(), ctx.pointerType()); // positional arguments
    bool uses_closure_env = !captures.empty();
    if (uses_closure_env) {
        argTypes.push_back(ctx.pointerType()); // closure environment
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(ctx.pointerType(), argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType,
                                                      llvm::Function::ExternalLinkage,
                                                      "func_overload_" + std::to_string(
                                                              Runtime::getInstance().nextId()),
                                                      *ctx.m_Module);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(*ctx.m_LLVMContext, "entry", function);

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
        auto &arg_expr = m_Args[arg_index++];
        llvm::AllocaInst *arg_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), nullptr, arg_expr.name());
        ctx.m_IRBuilder.CreateStore(&arg, arg_alloca);
        if (auto old_alloca = ctx.m_VariableMap.find(arg_expr.name()); old_alloca != ctx.m_VariableMap.end()) {
            shadowed_allocas[arg_expr.name()] = old_alloca->second;
        } else {
            shadowed_allocas[arg_expr.name()] = nullptr;
        }
        ctx.m_VariableMap[arg_expr.name()] = arg_alloca;
        loop_variable_storages.emplace_back(arg_alloca);
    }

    llvm::Argument *prev_closure_env = ctx.m_ClosureEnv;

    if (uses_closure_env) {
        llvm::Argument *closure_env_arg = function->getArg(m_Args.size());
        ctx.m_ClosureEnv = closure_env_arg;
    }

    // Create a recursion point
    llvm::BasicBlock *recursion_point = ctx.createBasicBlock("fn_recursion_point");
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

    if (uses_closure_env) {
        ctx.m_ClosureEnv = prev_closure_env;
    }

    ctx.m_CurrentFunction = prev_function;

    return function;
}

FunctionOverload FunctionOverload::parse(AnalyzerContext &ctx, const Object *form, bool is_eval_wrapper) {
    std::unordered_map<std::string, std::shared_ptr<LocalBindingExpr>> bindings_shadowed_in_fn;
    std::unordered_set<std::string> new_scope_bindings;
    ctx.m_CaptureUsedStack.emplace_back(false);
    ctx.m_StackFrameBindings.emplace_back();
    ctx.m_NumLocalsStack.emplace_back(0);

    const Object *arglist = tc_list_first(form);
    form = tc_list_next(form);

    if (arglist == nullptr || arglist->m_Type != ObjectType::LIST) {
        throw std::runtime_error("fn overload requires a list of arguments");
    }

    std::vector<FnArgExpr> args;
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
                throw std::runtime_error("Invalid use of & in argument list: & can only be used once"
                                         " and must be followed by a single vararg name");
            }
            varargs_state = VA_State::EXPECTING_VARARG_NAME;
            continue;
        } else if (varargs_state == VA_State::EXPECTING_VARARG_NAME) {
            varargs_state = VA_State::FOUND_VARARG;
        }

        if (new_scope_bindings.contains(arg_name)) {
            // Shadowing of another argument in the same argument list -> no need to store information about the shadowed
            // binding, which is the shadowed argument in this case
        } else {
            if (auto old_binding = ctx.m_ScopeBindings.find(arg_name);
                    old_binding != ctx.m_ScopeBindings.end()) {
                // The new binding shadows an existing binding in an outer function scope
                // -> store the old binding to restore it later
                bindings_shadowed_in_fn.emplace(arg_name, old_binding->second);
            } else {
                // The new argument does not shadow anything -> register it as a new scope binding to remove it later
                new_scope_bindings.emplace(arg_name);
            }
        }
        FnArgExpr arg_expr = FnArgExpr(arg_name, ctx.functionDepth(), ctx.currentLocalCount()++);
        args.emplace_back(arg_expr);

        ctx.currentStackFrameBindings().emplace(arg_name, std::make_shared<FnArgExpr>(arg_expr));
        ctx.m_ScopeBindings.insert_or_assign(arg_name, std::make_shared<FnArgExpr>(arg_expr));
    }

    if (varargs_state == VA_State::EXPECTING_VARARG_NAME) {
        throw std::runtime_error("Expected vararg name after & in argument list");
    }

    if (!is_eval_wrapper) {
        ctx.m_NumRecurArgsStack.emplace_back(args.size());
    }

    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        bool is_last = tc_list_next(form) == nullptr;
        // disable (recur) in the eval wrapper
        ExpressionMode last_mode = is_eval_wrapper ? ExpressionMode::EXPR : ExpressionMode::TAIL;
        const Object *expr = tc_list_first(form);
        body.emplace_back(SemanticAnalyzer::analyze(
                // discard return value of all but the last expression in the function body
                is_last ? last_mode : ExpressionMode::DISCARD, ctx, expr));
    }


    if (!is_eval_wrapper) {
        ctx.m_NumRecurArgsStack.pop_back();
    }
    // erase bindings introduced by the fn expression from the context
    for (const auto &var: new_scope_bindings) {
        ctx.currentStackFrameBindings().erase(var);
        ctx.m_ScopeBindings.erase(var);
    }
    // restore shadowed bindings in the context
    for (const auto &[name, binding_ref]: bindings_shadowed_in_fn) {
        ctx.currentStackFrameBindings()[name] = binding_ref;
        ctx.m_ScopeBindings[name] = binding_ref;
    }

    bool capture_used = ctx.m_CaptureUsedStack.back();

    ctx.m_NumLocalsStack.pop_back();
    ctx.m_StackFrameBindings.pop_back();
    ctx.m_CaptureUsedStack.pop_back();

    return FunctionOverload(std::move(args),
                            varargs_state == VA_State::FOUND_VARARG,
                            capture_used,
                            std::move(body));
}
