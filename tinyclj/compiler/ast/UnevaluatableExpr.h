#pragma once

#include "Expr.h"

class UnevaluatableExpr : public Expr {
public:
    Object *eval(Runtime &runtime) const final;
};
