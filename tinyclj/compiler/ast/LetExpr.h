#pragma once

#include <vector>
#include <tuple>

#include "compiler/AnalyzerContext.h"
#include "local-binding/LocalVarExpr.h"

class LetExpr : public UnevaluatableExpr {
    std::vector<std::pair<std::shared_ptr<LocalVarExpr>, AExpr>> m_Bindings;
    std::vector<AExpr> m_Body;
    bool m_IsLoop;
public:
    void emitIR(llvm::AllocaInst *dst, CodegenContext &ctx) const override;

    LetExpr(std::vector<std::pair<std::shared_ptr<LocalVarExpr>, AExpr>> bindings,
            std::vector<AExpr> body,
            bool isLoop);

    static AExpr parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);
};
