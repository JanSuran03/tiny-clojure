#include "TCClosure.h"

extern "C" {
Object *tc_closure_new(CallFn callThunk, void *env) {
    TCClosure *closure = new TCClosure{.m_Env = env};

    return new Object{
            .m_Data = closure,
            .m_Type = ObjectType::CLOSURE,
            .m_Call = callThunk
    };
}
}
