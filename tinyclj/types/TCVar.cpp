#include <cstring>
#include <stdexcept>

#include "runtime/Runtime.h"
#include "TCVar.h"

Object *tc_var_invoke(const Object *self, size_t argc, const Object **argv) {
    const TCVar *var = static_cast<const TCVar *>(self->m_Data);
    const Object *root = var->m_Root;
    if (root == nullptr) {
        throw std::runtime_error("Cannot invoke unbound var");
    }
    return root->m_Call(root, argc, argv);
}

extern "C" {
Object *tc_var_new(const char *name) {
    TCVar *var = new TCVar{.m_Name = strdup(name)};

    return Runtime::getInstance().createObject(ObjectType::VAR, var, tc_var_invoke);
}

const Object *tc_var_get_root(Object *var) {
    return static_cast<const TCVar *>(var->m_Data)->m_Root;
}

void tc_var_bind_root(Object *var, const Object *obj) {
    static_cast<TCVar *>(var->m_Data)->m_Root = obj;
}

bool tc_var_is_macroX(const Object *var) {
    return static_cast<const TCVar *>(var->m_Data)->m_IsMacro;
}

void tc_var_set_macroX(Object *var, bool is_macro) {
    static_cast<TCVar *>(var->m_Data)->m_IsMacro = is_macro;
}

}
