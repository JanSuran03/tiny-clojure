#pragma once

#include "Expr.h"

class UnevaluatableExpr : public Expr {
public:
    Object *eval() const final;
};
