#include "LetExpr.h"
#include "compiler/ast/local-binding/BindingExpr.h"
#include "compiler/ast/local-binding/LocalVarExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"
#include "types/TCInteger.h"
#include "types/TCSymbol.h"

EmitResult LetExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;
    /// EMIT let* ///
    if (!m_IsLoop) {
        std::unordered_map<std::string, Value *> shadowed_context_vars;
        for (const auto &[binding_var, binding_expr]: m_Bindings) {
            const std::string &binding_name = binding_var->name();
            if (ctx.m_VariableMap.contains(binding_name)) {
                shadowed_context_vars.emplace(binding_name, ctx.m_VariableMap[binding_name]);
            } else {
                shadowed_context_vars.emplace(binding_name, nullptr);
            }
            Value *binding_llvm_val = binding_expr->emitIR(ctx).value();
            ctx.m_VariableMap[binding_name] = binding_llvm_val;
        }
        EmitResult let_body_result = CompilerUtils::emitBody(m_Body, "let", ctx);
        // restore shadowed variables in the context
        for (const auto &[name, prev_value]: shadowed_context_vars) {
            if (prev_value) {
                ctx.m_VariableMap[name] = prev_value;
            } else {
                ctx.m_VariableMap.erase(name);
            }
        }
        return let_body_result;
    }

    /// EMIT loop* ///
    BasicBlock *recursion_point = ctx.createBasicBlock("loop_recursion_point");
    // Create phi node for loop arguments
    std::vector<PHINode *> phi_nodes;
    std::unordered_map<std::string, Value *> shadowed_context_vars;
    for (size_t i = 0; i < m_Bindings.size(); i++) {
        const std::string &binding_name = m_Bindings[i].first->name();
        PHINode *phi_node = PHINode::Create(ctx.pointerType(),
                                            0,
                                            "loop_arg_phi_" + std::to_string(i),
                                            recursion_point);
        phi_nodes.emplace_back(phi_node);
        Value *binding_llvm_val = m_Bindings[i].second->emitIR(ctx).value();
        // Store the initial loop arguments in the phi nodes
        // This needs to be done before altering the context with the loop argument bindings, since the loop
        // variables might shadow variables in the context that are used in the loop argument expressions, e.g.:
        // (loop (x x) ...)
        BasicBlock *current_block = ctx.m_IRBuilder.GetInsertBlock();
        phi_nodes[i]->addIncoming(binding_llvm_val, current_block);

        if (ctx.m_VariableMap.contains(binding_name) && !shadowed_context_vars.contains(binding_name)) {
            shadowed_context_vars.emplace(binding_name, ctx.m_VariableMap[binding_name]);
        } else {
            shadowed_context_vars.emplace(binding_name, nullptr);
        }
        ctx.m_VariableMap[binding_name] = phi_node;
    }
    ctx.m_LoopLabels.emplace_back(recursion_point, phi_nodes);

    ctx.m_IRBuilder.CreateBr(recursion_point);
    ctx.m_IRBuilder.SetInsertPoint(recursion_point);

    Value *loop_body_result = CompilerUtils::emitBody(m_Body, "loop", ctx).value();
    ctx.m_LoopLabels.pop_back();
    // restore shadowed variables in the context
    for (const auto &[name, prev_value]: shadowed_context_vars) {
        if (prev_value) {
            ctx.m_VariableMap[name] = prev_value;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }
    return loop_body_result;
}

LetExpr::LetExpr(std::vector<std::pair<std::shared_ptr<LocalVarExpr>, AExpr>> bindings,
                 std::vector<AExpr> body,
                 bool isLoop)
        : m_Bindings(std::move(bindings)),
          m_Body(std::move(body)),
          m_IsLoop(isLoop) {}

AExpr LetExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    bool is_loop = strcmp(tc_symbol_valueX(tc_list_first(form)), "loop*") == 0;
    ExpressionMode new_mode = is_loop ? ExpressionMode::TAIL : mode;
    form = tc_list_next(form); // consume let* / loop*
    const Object *bindings = tc_list_first(form);
    form = tc_list_next(form);
    if (bindings == nullptr || bindings->m_Type != ObjectType::LIST) {
        throw std::runtime_error("let requires a list of bindings");
    }

    tc_int_t bindings_len = static_cast<TCInteger *>(tc_list_length(bindings)->m_Data)->m_Value;
    if (bindings_len & 1) {
        throw std::runtime_error("let requires an even number of binding forms");
    }

    // Bindings shadowed in the let bindings that need to be restored after the let expression's scope ends.
    // To allow multiple bindings that shadow the same variable, we cannot store the old bindings in a vector,
    // we need a map.
    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> bindings_shadowed_in_let;
    // bindings introduced in the let expression that do not shadow any existing bindings
    std::unordered_set<std::string> new_scope_bindings;

    std::vector<std::pair<std::shared_ptr<LocalVarExpr>, AExpr>> parsed_bindings;
    for (bindings = tc_list_seq(bindings); bindings;) {
        const Object *binding_sym = tc_list_first(bindings);
        std::string binding_name = tc_symbol_valueX(binding_sym);
        bindings = tc_list_next(bindings);
        const Object *binding_val = tc_list_first(bindings);
        bindings = tc_list_next(bindings);
        if (binding_sym->m_Type != ObjectType::SYMBOL) {
            throw std::runtime_error("let binding name must be a symbol");
        }

        if (new_scope_bindings.contains(binding_name)) {
            // If the binding shadows an existing binding !!!within the same let expression!!!, the old binding
            // does not need to store additional information to restore the possibly old binding after the let
            // expression's scope ends - the information has possibly already been stored.
            // -> do nothing
        } else {
            if (auto old_binding = ctx.m_ScopeBindings.find(binding_name);
                    old_binding != ctx.m_ScopeBindings.end()) {
                // The new binding shadows an existing binding -> store the old binding to restore it later
                bindings_shadowed_in_let.emplace(binding_name, old_binding->second);
            } else {
                // The new binding does not shadow anything -> register it as a new scope binding to remove it later
                new_scope_bindings.emplace(binding_name);
            }
        }

        std::shared_ptr<LocalVarExpr> local_binding_expr = std::make_shared<LocalVarExpr>(
                binding_name,
                ctx.functionDepth());

        std::string binding_name_hint = std::string(is_loop ? "loop" : "let")
                .append("_binding_")
                .append(binding_name)
                .append("_")
                .append(std::to_string(Runtime::nextId()));

        parsed_bindings.emplace_back(local_binding_expr, SemanticAnalyzer::analyze(
                ExpressionMode::EXPR,
                ctx,
                binding_val,
                std::string (is_loop ? "loop_" : "let").append("_binding_")));
        // todo: unify value vs shared ptr vs unique ptr
        ctx.currentStackFrameBindings().emplace(binding_name, local_binding_expr);
        ctx.m_ScopeBindings.emplace(binding_name, local_binding_expr);
    }
    std::vector<AExpr> body;

    if (is_loop) {
        ctx.m_RecurFrames.emplace_back(parsed_bindings.size());
    }

    for (; form; form = tc_list_next(form)) {
        body.emplace_back(SemanticAnalyzer::analyze(tc_list_next(form) == nullptr ? new_mode : ExpressionMode::DISCARD,
                                                    ctx,
                                                    tc_list_first(form)));
    }

    // If the loop expression does not contain a recur expression, we can treat it as a let expression
    // and avoid emitting the additional loop machinery in the IR.
    is_loop = is_loop && ctx.currentRecurContext().m_IsReferenced;

    if (is_loop) {
        ctx.m_RecurFrames.pop_back();
    }

    // erase new bindings introduced in the let expressions
    for (const std::string &binding_name: new_scope_bindings) {
        ctx.currentStackFrameBindings().erase(binding_name);
        ctx.m_ScopeBindings.erase(binding_name);
    }
    // restore shadowed bindings in the context
    for (const auto &[binding_name, binding_ref]: bindings_shadowed_in_let) {
        ctx.currentStackFrameBindings()[binding_name] = binding_ref;
        ctx.m_ScopeBindings[binding_name] = binding_ref;
    }

    return std::make_unique<LetExpr>(std::move(parsed_bindings),
                                     std::move(body),
                                     is_loop);
}
