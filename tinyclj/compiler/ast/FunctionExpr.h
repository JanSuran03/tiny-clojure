#pragma once

#include "Expr.h"
#include "UnevaluatableExpr.h"

class FunctionExpr : public Expr {
    std::string m_Name;
    std::vector<std::string> m_Args;
    std::set<std::string> m_Captures = {}; // for now
    std::vector<AExpr> m_Body;

    void compile(CompilerContext &ctx) const;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    FunctionExpr(std::string name, std::vector<std::string> args, std::vector<AExpr> body);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
