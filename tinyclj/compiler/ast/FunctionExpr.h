#pragma once

#include <optional>

#include "compiler/AnalyzerContext.h"
#include "Expr.h"
#include "FunctionOverload.h"
#include "UnevaluatableExpr.h"

class FunctionExpr : public Expr {
    std::string m_Name;
    std::string m_StubName = m_Name + "__stub";
    std::string m_VtableName = m_Name + "__vtable";
    std::unordered_map<size_t, FunctionOverload> m_Overloads;
    std::optional<FunctionOverload> m_VariadicOverload;
    Captures m_Captures;
    /** A set of all global variables referenced by this function that need to be emitted as global variables
     * for the function's LLVM module.
     */
    std::unordered_set<std::string> m_ReferencedGlobalNames;
    std::unordered_set<std::string> m_ModuleImports;

    std::string compile() const;

    bool isClosure() const;

public:
    EmitResult emitIR(CodegenContext &ctx) const override;

    const Object *eval() const override;

    FunctionExpr(std::string name,
                 std::unordered_map<size_t, FunctionOverload> overloads,
                 std::optional<FunctionOverload> variadic_overload,
                 Captures captures,
                 std::unordered_set<std::string> referenced_global_names,
                 std::unordered_set<std::string> module_imports);

    static AExpr parse(ExpressionMode mode,
                       AnalyzerContext &ctx,
                       const Object *form,
                       const std::optional<std::string> &nameHint);
};
