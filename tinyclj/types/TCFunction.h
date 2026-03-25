#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCFunction {
    char *m_Name;
};

extern "C" {
Object *tc_function_new(CallFn callStub, const char *name);
}
