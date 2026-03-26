#pragma once

#include "UnevaluatableExpr.h"

class FunctionOverload {
    using Captures = std::unordered_map<std::string, int>;
    std::vector<std::string> m_Args;
    bool m_IsVariadic;
    bool m_UsesClosureEnv;
    std::vector<AExpr> m_Body;

    llvm::Function *compile(CompilerContext &ctx, const Captures &captures) const;

public:
    friend class FunctionExpr;

    FunctionOverload(std::vector<std::string> args,
                     bool isVariadic,
                     bool usesClosureEnv,
                     std::vector<AExpr> body);

    static FunctionOverload parse(CompilerContext &ctx, const Object *form, bool is_eval_wrapper);
};
