#include "compiler/ast/local-binding/LocalBindingExpr.h"
#include "LetExpr.h"
#include "compiler/ast/local-binding/LocalVarExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCList.h"
#include "types/TCInteger.h"
#include "types/TCSymbol.h"

void LetExpr::emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const {
    std::unordered_map<std::string, llvm::AllocaInst *> shadowed_allocas;
    std::vector<llvm::AllocaInst *> variable_storages;
    for (const auto &[name, value]: m_Bindings) {
        // allocate space for a 64-bit pointer as every runtime object is a pointer to the Object struct
        llvm::AllocaInst *alloca = ctx.m_IRBuilder.CreateAlloca(
                ctx.pointerType(),
                nullptr,
                name);
        value->emitIR(alloca, ctx);
        // if the variable is shadowing another variable, save the old alloca to restore it later
        if (auto old_alloca = ctx.m_VariableMap.find(name); old_alloca != ctx.m_VariableMap.end()) {
            shadowed_allocas[name] = old_alloca->second;
        } else {
            shadowed_allocas[name] = nullptr;
        }
        ctx.m_VariableMap[name] = alloca;
        variable_storages.emplace_back(alloca);
    }

    // create a recursion point if this is a loop expression
    llvm::BasicBlock *recursion_point = nullptr;
    if (m_IsLoop) {
        recursion_point = ctx.createBasicBlock("loop_recursion_point");
        ctx.m_IRBuilder.CreateBr(recursion_point);
        ctx.m_IRBuilder.SetInsertPoint(recursion_point);
        ctx.m_LoopLabels.emplace_back(recursion_point, std::move(variable_storages));
    }

    CompilerUtils::emitBody(m_Body, m_IsLoop ? "let" : "loop", dst, ctx);

    if (m_IsLoop) {
        ctx.m_LoopLabels.pop_back();
    }

    // restore shadowed variables in the context
    for (const auto &[name, alloca]: shadowed_allocas) {
        if (alloca) {
            ctx.m_VariableMap[name] = alloca;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }
}

LetExpr::LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings,
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
    if (bindings == nullptr || tinyclj_object_get_type(bindings) != ObjectType::LIST) {
        throw std::runtime_error("let requires a list of bindings");
    }

    tc_int_t bindings_len = static_cast<TCInteger *>(tc_list_length(bindings)->m_Data)->m_Value;
    if (bindings_len & 1) {
        throw std::runtime_error("let requires an even number of binding forms");
    }

    // Bindings shadowed in the let bindings that need to be restored after the let expression's scope ends.
    // To allow multiple bindings that shadow the same variable, we cannot store the old bindings in a vector,
    // we need a map.
    std::unordered_map<std::string, std::shared_ptr<LocalBindingExpr>> bindings_shadowed_in_let;
    // bindings introduced in the let expression that do not shadow any existing bindings
    std::unordered_set<std::string> new_scope_bindings;

    std::vector<std::tuple<std::string, AExpr>> parsed_bindings;
    for (bindings = tc_list_seq(bindings); bindings;) {
        const Object *binding_sym = tc_list_first(bindings);
        std::string binding_name = tc_symbol_valueX(binding_sym);
        bindings = tc_list_next(bindings);
        const Object *binding_val = tc_list_first(bindings);
        bindings = tc_list_next(bindings);
        if (tinyclj_object_get_type(binding_sym) != ObjectType::SYMBOL) {
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

        parsed_bindings.emplace_back(binding_name, SemanticAnalyzer::analyze(
                ExpressionMode::EXPR,
                ctx,
                binding_val));

        std::shared_ptr<LocalBindingExpr> local_binding_expr = std::make_shared<LocalVarExpr>(
                binding_name,
                ctx.functionDepth(),
                ctx.currentLocalCount()++);
        ctx.currentStackFrameBindings().emplace(binding_name, local_binding_expr);
        ctx.m_ScopeBindings.emplace(binding_name, local_binding_expr);
    }
    std::vector<AExpr> body;

    if (is_loop) {
        ctx.m_NumRecurArgsStack.emplace_back(parsed_bindings.size());
    }

    for (; form; form = tc_list_next(form)) {
        body.emplace_back(SemanticAnalyzer::analyze(tc_list_next(form) == nullptr ? new_mode : ExpressionMode::DISCARD,
                                                    ctx,
                                                    tc_list_first(form)));
    }

    if (is_loop) {
        ctx.m_NumRecurArgsStack.pop_back();
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
