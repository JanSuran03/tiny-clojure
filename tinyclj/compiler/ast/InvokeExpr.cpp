#include "DynamicInvokeExpr.h"
#include "InvokeExpr.h"
#include "StaticInvokeExpr.h"
#include "VarDerefExpr.h"
#include "compiler/CompilerUtils.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/rt.h"
#include "runtime/Runtime.h"
#include "types/TCFunction.h"
#include "types/TCList.h"

const Object *InvokeExpr::eval() const {
    std::vector<const Object *> evaled_args;
    for (const auto &arg: m_InvokeArgs) {
        evaled_args.emplace_back(arg->eval());
    }
    return evalInvoke(evaled_args);
}

InvokeExpr::InvokeExpr(std::vector<AExpr> invokeArgs)
        : m_InvokeArgs(std::move(invokeArgs)) {}

AExpr InvokeExpr::parse(ExpressionMode _, AnalyzerContext &ctx, const Object *form) {
    AExpr invokeTarget = SemanticAnalyzer::analyze(ctx, tc_list_first(form));

    std::vector<AExpr> invokeArgs;
    std::vector<LocalVarExpr> invoke_args_locals_vars;
    for (const Object *args = tc_list_next(form); args; args = tc_list_next(args)) {
        invokeArgs.emplace_back(SemanticAnalyzer::analyze(ctx, tc_list_first(args)));
    }

    Runtime &rt = Runtime::getInstance();
    if (rt.m_DirectLinking) {
        // Grab the current value of the var. If the object has type ObjectType::FUNCTION,
        // generate a direct call instead. This bypasses the var dereference at runtime.
        if (auto var_expr = dynamic_cast<VarDerefExpr *>(invokeTarget.get())) {
            const Object *callable_obj = var_expr->eval();
            if (callable_obj != nullptr && callable_obj->m_Type == ObjectType::FUNCTION) {
                std::string function_name = static_cast<const TCFunction *>(callable_obj->m_Data)->m_Name;
                if (!function_name.starts_with("tinyclj_rt")) {
                    ctx.m_ModuleImportsStack.back().emplace(function_name);
                }
                return std::make_unique<StaticInvokeExpr>(callable_obj, std::move(invokeArgs));
            }
        }
    }

    return std::make_unique<DynamicInvokeExpr>(std::move(invokeTarget),
                                               std::move(invokeArgs));
}
