#include <utility>

#include "DefExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/Runtime.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

DefExpr::DefExpr(Object *var, AExpr value)
        : m_Var(var),
          m_Value(std::move(value)) {}

EmitResult DefExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;

    // emit a call to tc_var_bind_root with the variable and the value
    FunctionType *bind_var_fn_type = FunctionType::get(
            Type::getVoidTy(*ctx.m_LLVMContext),
            {ctx.pointerType(), ctx.pointerType()},
            false);
    FunctionCallee bind_var_fn = ctx.m_Module->getOrInsertFunction("tc_var_bind_root", bind_var_fn_type);

    EmitResult value_to_bind = m_Value->emitIR(ctx);
    // todo: instead use the var name directly lol, why use static cast
    Value *llvm_var_ptr = CompilerUtils::emitGlobalVar(ctx, static_cast<TCVar *>(m_Var->m_Data)->m_Name);
    // todo: make tc_Var_bind_root not void and return its result directly
    ctx.m_IRBuilder.CreateCall(bind_var_fn, {llvm_var_ptr, value_to_bind.value()});
    return llvm_var_ptr;
}

const Object *DefExpr::eval() const {
    const Object *res = m_Value->eval();
    tc_var_bind_root(m_Var, res);
    return m_Var;
}

AExpr DefExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
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

            // allow recursive use (uninitialized = always nullptr, at least for now)
            auto var_name = static_cast<TCSymbol *>(name->m_Data)->m_Name;
            ctx.m_ReferencedGlobalNamesStack.back().emplace(var_name);
            Runtime::getInstance().getAotEngine().startLoading(var_name);
            auto var = Runtime::getInstance().declareVar(var_name);
            AExpr init_expr = SemanticAnalyzer::analyze(mode, ctx, init, var_name);
            Runtime::getInstance().getAotEngine().finishLoading(var_name);

            return std::make_unique<DefExpr>(var, std::move(init_expr));
        }
        default:
            throw std::runtime_error("'def form must contain 2 or 3 items: (def name) or (def name value)");
    }
}
