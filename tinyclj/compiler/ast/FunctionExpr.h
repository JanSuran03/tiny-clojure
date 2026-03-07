#pragma once

#include "Expr.h"

class FunctionExpr : public Expr {
    std::string m_Name;
    std::vector<std::string> m_Args;
    std::set<std::string> m_Captures;
    AExpr m_Body;

public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    FunctionExpr(std::string name, std::vector<std::string> args, AExpr body);
};
