#include "BindingExpr.h"
#include "runtime/Runtime.h"

BindingExpr::BindingExpr(std::string name) : m_Name(std::move(name)) {}

const std::string &BindingExpr::name() const {
    return m_Name;
}
