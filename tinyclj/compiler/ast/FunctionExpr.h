#pragma once

#include "Expr.h"
#include "UnevaluatableExpr.h"

class FunctionExpr : public Expr {
    using Captures = std::unordered_map<std::string, int>;
    std::string m_Name;
    std::vector<std::string> m_Args;
    Captures m_Captures;
    std::vector<AExpr> m_Body;

    void compile(CompilerContext &ctx) const;

    bool isClosure();
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    FunctionExpr(std::string name,
                 std::vector<std::string> args,
                 Captures captures,
                 std::vector<AExpr> body);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
