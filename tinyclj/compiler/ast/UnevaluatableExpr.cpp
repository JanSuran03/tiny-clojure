#include "UnevaluatableExpr.h"

Object *UnevaluatableExpr::eval() const {
    auto &ti = typeid(*this);
    throw std::runtime_error(std::string("An expression of type")
                                     .append(ti.name())
                                     .append(" cannot be evaluated"));
}
