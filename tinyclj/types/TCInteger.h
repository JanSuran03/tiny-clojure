#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCInteger {
    tc_int_t m_Value;

    static const Object *equals(const Object *self, const Object *other);

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;

    static void init();

    static constexpr tc_int_t PREALLOCATED_MIN = -128;
    static constexpr tc_int_t PREALLOCATED_MAX = 127;
    static Object st_PreallocatedIntegers[PREALLOCATED_MAX - PREALLOCATED_MIN + 1];

    static Object *allocate_integer(tc_int_t value);
};

extern "C" {
Object *tc_integer_new(tc_int_t value);
}
