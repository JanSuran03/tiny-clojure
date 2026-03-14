#include "TCVar.h"

extern "C" {
TCVar *tc_var_new() {
    return new TCVar;
}

const Object *tc_var_get_root(TCVar *var) {
    return var->m_Root;
}

void tc_var_bind_root(TCVar *var, const Object *obj) {
    var->m_Root = obj;
}
}
