#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCInteger {
    tc_int_t m_Value;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_integer_new(tc_int_t value);
}
