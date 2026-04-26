#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCDouble {
    tc_double_t m_Value;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_double_new(tc_double_t value);
}
