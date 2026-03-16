#include "DefExpr.h"

#include <utility>
#include "SemanticAnalyzer.h"
#include "Runtime.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

DefExpr::DefExpr(std::string name, AExpr value)
        : m_Name(std::move(name)),
          m_Value(std::move(value)) {}

void DefExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("DefExpr::emitIR not implemented");
}

Object *DefExpr::eval(Runtime &runtime) const {
    Object *res = m_Value->eval(runtime);
    tc_var_bind_root(runtime.getVar(m_Name), res);
    return res;
}

AExpr DefExpr::parse(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
    TCList *list = static_cast<TCList *>(form->m_Data);

    // no declaration possible: need form (def name value)
    switch (list->m_Length) {
        case 2:
        case 3: {
            bool has_init = list->m_Length == 3;
            const Object *args = tc_list_next(form);
            const Object *name = tc_list_first(args);
            if (name == nullptr || name->m_Type != ObjectType::SYMBOL) {
                throw std::runtime_error("'def form must def a symbol.");
            }
            const Object *init = nullptr;
            if (has_init) {
                args = tc_list_next(args);
                init = tc_list_first(args);
            }
            AExpr init_expr = SemanticAnalyzer::analyze(mode, ctx, init);
            // after init_expr analysis - don't allow recursive use of a (technically) uninitialized variable
            // (it is initialized to nullptr)
            auto var_name = static_cast<TCSymbol *>(name->m_Data)->m_Value;
            ctx.m_RuntimeRef.declareVar(var_name);
            return std::make_unique<DefExpr>(var_name, std::move(init_expr));
        }
        default:
            throw std::runtime_error("'def form must contain 2 or 3 items: (def name) or (def name value)");
    }
}
