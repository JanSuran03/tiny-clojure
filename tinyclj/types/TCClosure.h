#pragma once

#include "Object.h"
#include "../tcdef.h"

struct TCClosure {
    const Object **m_Env;
};

extern "C" {
Object *tc_closure_new(CallFn callThunk, const Object **env);

Object **tc_closure_allocate_env(size_t numCaptures);

void *tc_closure_get_envX(Object *obj);
}
