#pragma once

#include <vector>
#include <tuple>

#include "Expr.h"

class LetExpr : public Expr {
    std::vector<std::tuple<std::string, AExpr>> m_Bindings;
    AExpr m_Body;
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    LetExpr(std::vector<std::tuple<std::string, AExpr>> bindings, AExpr body);

    // todo: implement static analysis
    // static AExpr parse(...);
};
