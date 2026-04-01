#pragma once

#include "UnevaluatableExpr.h"
#include "compiler/AnalyzerContext.h"
#include "local-binding/FnArgExpr.h"

class FunctionOverload {
    std::vector<FnArgExpr> m_Args;
    std::vector<unsigned> m_InvokeArgCounts;
    bool m_IsVariadic;
    bool m_UsesClosureEnv;
    Captures m_Captures;
    std::vector<AExpr> m_Body;

    llvm::Function *compile(CodegenContext &ctx, const Captures &captures) const;

public:
    friend class FunctionExpr;

    FunctionOverload(std::vector<FnArgExpr> args,
                     std::vector<unsigned> invokeArgCounts,
                     bool isVariadic,
                     bool usesClosureEnv,
                     std::vector<AExpr> body);

    static FunctionOverload parse(AnalyzerContext &ctx, const Object *form, bool is_eval_wrapper);
};
