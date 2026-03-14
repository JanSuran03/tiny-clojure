/*
#pragma once

#include "Object.h"

struct TCVar {
    const Object *m_Root = nullptr;
};

extern "C" {
Object *tc_var_new();

const Object *tc_var_get_root();

const Object *tc_var_bind_root(const Object *obj);
}
*/

#pragma once

#include "Object.h"

struct TCVar {
    const Object *m_Root = nullptr;
};

extern "C" {
TCVar *tc_var_new();

const Object *tc_var_get_root(TCVar *var);

void tc_var_bind_root(TCVar *var, const Object *obj);
}
