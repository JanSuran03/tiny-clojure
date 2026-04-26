#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCFunction {
    char *m_Name;

    static const Object *toString(const Object *self);
};

extern "C" {
void tc_function_init_vtable(MethodTable *methodTable, CallFn callStub);

Object *tc_function_new(const MethodTable *methodTable, const char *name);
}
