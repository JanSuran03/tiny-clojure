#pragma once

#include "compiler/ast/UnevaluatableExpr.h"

class BindingExpr : public UnevaluatableExpr {
protected:
    std::string m_Name;
public:
    BindingExpr(std::string name);

    const std::string &name() const;

    virtual AExpr clone() const = 0;
};
