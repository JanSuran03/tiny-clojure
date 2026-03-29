#pragma once

#include <optional>

#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "FunctionOverload.h"
#include "UnevaluatableExpr.h"

class FunctionExpr : public Expr {
    std::string m_Name;
    std::string m_StubName = m_Name + "__stub";
    std::unordered_map<size_t, FunctionOverload> m_Overloads;
    std::optional<FunctionOverload> m_VariadicOverload;
    Captures m_Captures;

    void compile(CodegenContext &ctx) const;

    bool isClosure() const;

public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    Object *eval(Runtime &runtime) const override;

    FunctionExpr(std::string name,
                 std::unordered_map<size_t, FunctionOverload> overloads,
                 std::optional<FunctionOverload> variadic_overload,
                 Captures captures);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
