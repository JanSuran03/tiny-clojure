#pragma once

#include "Object.h"
#include "../tcdef.h"

struct TCClosure {
    void *m_Env;
};

extern "C" {
Object *tc_closure_new(CallFn callThunk, void *env);
}
