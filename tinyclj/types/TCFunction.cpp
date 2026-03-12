#include <cstring>

#include "TCFunction.h"

extern "C" {
Object *tc_function_new(CallFn callThunk, const char *name) {
    TCFunction *function = new TCFunction{.m_Name = strdup(name)};

    return new Object{
            .m_Data = function,
            .m_Type = ObjectType::FUNCTION,
            .m_Call = callThunk
    };
}
}