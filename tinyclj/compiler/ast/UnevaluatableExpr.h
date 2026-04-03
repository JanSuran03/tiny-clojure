#pragma once

#include "Expr.h"

class UnevaluatableExpr : public Expr {
public:
    const Object *eval() const final;
};
