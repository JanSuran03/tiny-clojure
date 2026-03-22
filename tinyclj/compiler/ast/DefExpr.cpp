#include <utility>

#include "DefExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "Runtime.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

DefExpr::DefExpr(Object *var, AExpr value)
        : m_Var(var),
          m_Value(std::move(value)) {}

void DefExpr::emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const {
    using namespace llvm;

    // emit a call to tc_var_bind_root with the variable and the value
    FunctionType *bind_var_fn_type = FunctionType::get(
            Type::getVoidTy(ctx.m_LLVMContext),
            {ctx.pointerType(), ctx.pointerType()},
            false);
    FunctionCallee bind_var_fn = ctx.m_Module.getOrInsertFunction("tc_var_bind_root", bind_var_fn_type);

    llvm::AllocaInst *def_value_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), nullptr, "def_result");
    m_Value->emitIR(def_value_alloca, ctx);
    Value *llvm_var_ptr = CompilerUtils::emitObjectPtr(m_Var, ctx);
    Value *value_to_bind = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), def_value_alloca, "def_value");
    ctx.m_IRBuilder.CreateCall(bind_var_fn, {llvm_var_ptr, value_to_bind});
    if (dst != nullptr) {
        ctx.m_IRBuilder.CreateStore(llvm_var_ptr, dst);
    }
}

Object *DefExpr::eval(Runtime &runtime) const {
    Object *res = m_Value->eval(runtime);
    tc_var_bind_root(m_Var, res);
    return m_Var;
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

            // allow recursive use (uninitialized = always nullptr)
            auto var_name = static_cast<TCSymbol *>(name->m_Data)->m_Name;
            auto var = ctx.m_RuntimeRef.declareVar(var_name);
            AExpr init_expr = SemanticAnalyzer::analyze(mode, ctx, init);

            return std::make_unique<DefExpr>(var, std::move(init_expr));
        }
        default:
            throw std::runtime_error("'def form must contain 2 or 3 items: (def name) or (def name value)");
    }
}
