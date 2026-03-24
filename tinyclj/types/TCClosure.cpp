#include "Runtime.h"
#include "TCClosure.h"

extern "C" {
Object *tc_closure_new(CallFn callStub, const Object **env, size_t numCaptures) {
    TCClosure *closure = new TCClosure{.m_Env = env, .m_NumCaptures = numCaptures};

    return Runtime::getInstance().createObject(ObjectType::CLOSURE, closure, callStub);
}

Object **tc_closure_allocate_env(size_t numCaptures) {
    return new Object *[numCaptures];
}

void *tc_closure_get_envX(Object *obj) {
    return static_cast<TCClosure *>(obj->m_Data)->m_Env;
}
}
