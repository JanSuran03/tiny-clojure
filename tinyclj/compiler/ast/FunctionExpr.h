#pragma once

#include <optional>

#include "Expr.h"
#include "FunctionOverload.h"
#include "UnevaluatableExpr.h"

class FunctionExpr : public Expr {
    using Captures = std::unordered_map<std::string, int>;
    std::string m_Name;
    std::string m_StubName = m_Name + "__stub";
    std::unordered_map<size_t, FunctionOverload> m_Overloads;
    std::optional<FunctionOverload> m_VariadicOverload;
    Captures m_Captures;

    void compile(CompilerContext &ctx) const;

    bool isClosure() const;

public:
    void emitIR(llvm::AllocaInst *dst, CompilerContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    FunctionExpr(std::string name,
                 std::unordered_map<size_t, FunctionOverload> overloads,
                 std::optional<FunctionOverload> variadic_overload,
                 Captures captures);

    static AExpr parse(ExpressionMode mode, CompilerContext &ctx, const Object *form);
};
