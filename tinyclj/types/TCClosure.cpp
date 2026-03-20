#include "TCClosure.h"

extern "C" {
Object *tc_closure_new(CallFn callThunk, const Object **env) {
    TCClosure *closure = new TCClosure{.m_Env = env};

    return new Object{
            .m_Data = closure,
            .m_Type = ObjectType::CLOSURE,
            .m_Call = callThunk
    };
}

Object **tc_closure_allocate_env(size_t numCaptures) {
    return new Object *[numCaptures];
}

void *tc_closure_get_envX(Object *obj) {
    return static_cast<TCClosure *>(obj->m_Data)->m_Env;
}
}
