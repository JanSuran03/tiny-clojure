#include "ASTUtils.h"
#include "LetExpr.h"
#include "SemanticAnalyzer.h"
#include "types/TCList.h"
#include "types/TCInteger.h"
#include "types/TCSymbol.h"

void LetExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    std::unordered_map<std::string, llvm::AllocaInst *> shadowed_allocas;
    for (const auto &[name, value]: m_Bindings) {
        // allocate space for a 64-bit pointer as every runtime object is a pointer to the Object struct
        llvm::AllocaInst *alloca = ctx.m_IRBuilder.CreateAlloca(
                ctx.objectPointerType(),
                nullptr,
                name);
        value->emitIR(ExpressionMode::EXPRESSION, alloca, ctx);
        // if the variable is shadowing another variable, save the old alloca to restore it later
        if (auto old_alloca = ctx.m_VariableMap.find(name); old_alloca != ctx.m_VariableMap.end()) {
            shadowed_allocas[name] = old_alloca->second;
        } else {
            shadowed_allocas[name] = nullptr;
        }
        ctx.m_VariableMap[name] = alloca;
    }
    AstUtils::emitBody(m_Body, "let", mode, dst, ctx);

    // restore shadowed variables in the context
    for (const auto &[name, alloca]: shadowed_allocas) {
        if (alloca) {
            ctx.m_VariableMap[name] = alloca;
        } else {
            ctx.m_VariableMap.erase(name);
        }
    }
}

LetExpr::LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, std::vector<AExpr> body)
        : m_Bindings(std::move(bindings)),
          m_Body(std::move(body)) {}

AExpr LetExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    form = tc_list_next(form); // consume 'let*
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
            ctx.m_LocalBindings.insert(binding_name);
            new_scope_bindings.push_back(binding_name);
        }

        parsed_bindings.emplace_back(binding_name, SemanticAnalyzer::analyze(
                ExpressionMode::EXPRESSION,
                ctx,
                binding_val));

        ctx.m_CurrentFunctionFrame->m_Locals.emplace(binding_name);
    }
    std::vector<AExpr> body;
    for (; form; form = tc_list_next(form)) {
        body.push_back(SemanticAnalyzer::analyze(tc_list_next(form) == nullptr ? mode : ExpressionMode::STATEMENT,
                                                 ctx,
                                                 tc_list_first(form)));
    }

    for (const std::string &binding_name: new_scope_bindings) {
        ctx.m_LocalBindings.erase(binding_name);
        ctx.m_CurrentFunctionFrame->m_Locals.erase(binding_name);
    }

    return std::make_unique<LetExpr>(std::move(parsed_bindings), std::move(body));
}
