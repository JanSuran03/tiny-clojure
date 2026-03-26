#include "compiler/CompilerUtils.h"
#include "LetExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCList.h"
#include "types/TCInteger.h"
#include "types/TCSymbol.h"

void LetExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
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

AExpr LetExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
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

    // New scope variables that will be used for further resolution of symbols in let expression body
    // (or later in the bindings vector in the same let expression). These need to be removed from the
    // available context after parsing this let expression.
    std::vector<std::string> new_scope_bindings;

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
        // if the variable is shadowed, don't do anything
        if (!ctx.m_LocalBindings.contains(binding_name)) {
            new_scope_bindings.push_back(binding_name);
        }

        parsed_bindings.emplace_back(binding_name, SemanticAnalyzer::analyze(
                ExpressionMode::EXPR,
                ctx,
                binding_val));

        ctx.m_LocalBindings.insert(binding_name);
        ctx.m_StackFrameBindings.back().insert(binding_name);
    }
    std::vector<AExpr> body;

    size_t old_num_recur_args = ctx.m_NumRecurArgs;
    if (is_loop) {
        ctx.m_NumRecurArgs = parsed_bindings.size();
    }

    for (; form; form = tc_list_next(form)) {
        body.emplace_back(SemanticAnalyzer::analyze(tc_list_next(form) == nullptr ? new_mode : ExpressionMode::DISCARD,
                                                    ctx,
                                                    tc_list_first(form)));
    }

    if (is_loop) {
        ctx.m_NumRecurArgs = old_num_recur_args;
    }

    for (const std::string &binding_name: new_scope_bindings) {
        ctx.m_LocalBindings.erase(binding_name);
        ctx.m_StackFrameBindings.back().erase(binding_name);
    }

    return std::make_unique<LetExpr>(std::move(parsed_bindings),
                                     std::move(body),
                                     is_loop);
}
