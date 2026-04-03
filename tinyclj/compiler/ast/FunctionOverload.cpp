#include "FunctionOverload.h"
#include "local-binding/CapturedLocalExpr.h"
#include "local-binding/FnArgExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionOverload::FunctionOverload(std::vector<FnArgExpr> args,
                                   std::vector<unsigned> invokeArgCounts,
                                   bool isVariadic,
                                   bool usesClosureEnv,
                                   std::vector<AExpr> body)
        : m_Args(std::move(args)),
          m_InvokeArgCounts(std::move(invokeArgCounts)),
          m_IsVariadic(isVariadic),
          m_UsesClosureEnv(usesClosureEnv),
          m_Body(std::move(body)) {}

llvm::Function *FunctionOverload::compile(CodegenContext &ctx, const Captures &captures) const {
    using namespace llvm;

    std::vector<Type *> argTypes(m_Args.size(), ctx.pointerType()); // positional arguments
    bool uses_closure_env = !captures.empty();
    if (uses_closure_env) {
        argTypes.push_back(ctx.pointerType()); // closure environment
    }

    FunctionType *funcType = FunctionType::get(ctx.pointerType(), argTypes, false);
    Function *function = Function::Create(funcType,
                                          Function::ExternalLinkage,
                                          "func_overload_" + std::to_string(
                                                  Runtime::getInstance().nextId()),
                                          *ctx.m_Module);

    Function *prev_function = ctx.m_CurrentFunction;
    ctx.m_CurrentFunction = function;

    BasicBlock *entryBlock = ctx.createBasicBlock("entry");

    ctx.m_IRBuilder.SetInsertPoint(entryBlock);

    // allocate space for each invoke argv inside the current function frame
    ctx.m_InvokeArgvAllocasStack.emplace_back();
    auto &current_invoke_argv_allocas = ctx.currentInvokeArgvAllocas();
    for (size_t i = 0; i < m_InvokeArgCounts.size(); i++) {
        unsigned argc = m_InvokeArgCounts[i];
        AllocaInst *argv_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(),
                                                               ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext),
                                                                                argc,
                                                                                false),
                                                               "invoke_argv_alloca_" + std::to_string(i));
        current_invoke_argv_allocas.emplace_back(argv_alloca);
    }

    // Create an implicit tail recursion point
    BasicBlock *recursion_point = ctx.createBasicBlock("fn_recursion_point");
    // Create phi nodes for tail recursion arguments
    std::vector<PHINode *> phi_nodes;
    std::unordered_map<std::string, Value *> shadowed_context_vars;
    for (size_t i = 0; i < m_Args.size(); i++) {
        PHINode *phi_node = PHINode::Create(ctx.pointerType(),
                                            0,
                                            "fn_tailrec_phi_" + std::to_string(i),
                                            recursion_point);
        phi_nodes.emplace_back(phi_node);

        // bind the function argument to a variable in the context
        const std::string &arg_name = m_Args[i].name();
        // if the argument shadows an existing variable (but not another function argument), store
        // the old variable to restore it later and shadow it with the new argument variable
        if (ctx.m_VariableMap.contains(arg_name) && !shadowed_context_vars.contains(arg_name)) {
            shadowed_context_vars.emplace(arg_name, ctx.m_VariableMap[arg_name]);
        } else {
            shadowed_context_vars.emplace(arg_name, nullptr);
        }
        ctx.m_VariableMap[arg_name] = phi_node;
    }
    ctx.m_LoopLabels.emplace_back(recursion_point, phi_nodes);

    ctx.m_IRBuilder.CreateBr(recursion_point);
    ctx.m_IRBuilder.SetInsertPoint(recursion_point);

    // store the initial function's argument values in the phi nodes
    // Cannot iterate over function->args() if this overload uses the closure environment! - the closure
    // environment argument is implicit and cannot be rewritten with recur.
    for (size_t i = 0; i < m_Args.size(); i++) {
        Argument *arg = function->getArg(i);
        phi_nodes[i]->addIncoming(arg, entryBlock);
    }

    Argument *prev_closure_env = ctx.m_ClosureEnv;

    if (uses_closure_env) {
        Argument *closure_env_arg = function->getArg(m_Args.size());
        ctx.m_ClosureEnv = closure_env_arg;
    }

    auto function_return_value = CompilerUtils::emitBody(m_Body, "fn", ctx);
    ctx.m_IRBuilder.CreateRet(function_return_value.value());

    ctx.m_LoopLabels.pop_back();

    // restore shadowed variables in the context
    for (const auto &[name, prev_value]: shadowed_context_vars) {
        if (prev_value) {
            ctx.m_VariableMap[name] = prev_value;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }

    if (uses_closure_env) {
        ctx.m_ClosureEnv = prev_closure_env;
    }

    ctx.m_CurrentFunction = prev_function;

    ctx.m_InvokeArgvAllocasStack.pop_back();

    return function;
}

FunctionOverload FunctionOverload::parse(AnalyzerContext &ctx, const Object *form, bool is_eval_wrapper) {
    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> bindings_shadowed_in_fn;
    std::unordered_set<std::string> new_scope_bindings;
    ctx.m_CaptureUsedStack.emplace_back(false);
    ctx.m_StackFrameBindings.emplace_back();
    ctx.m_InvokeArgCountsStack.emplace_back();

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
        if (arg->m_Type != ObjectType::SYMBOL) {
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
        FnArgExpr arg_expr = FnArgExpr(arg_name, ctx.functionDepth());
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

    std::vector<unsigned> invoke_arg_counts = std::move(ctx.m_InvokeArgCountsStack.back());
    bool capture_used = ctx.m_CaptureUsedStack.back();

    ctx.m_InvokeArgCountsStack.pop_back();
    ctx.m_StackFrameBindings.pop_back();
    ctx.m_CaptureUsedStack.pop_back();

    return FunctionOverload(std::move(args),
                            std::move(invoke_arg_counts),
                            varargs_state == VA_State::FOUND_VARARG,
                            capture_used,
                            std::move(body));
}
