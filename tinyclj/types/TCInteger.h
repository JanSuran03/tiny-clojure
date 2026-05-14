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

    static tc_int_t st_PreallocatedMin;
    static tc_int_t st_PreallocatedMax;
    static bool st_PreallocationEnabled;
    static Object *st_PreallocatedIntegers;

    static Object *allocate_integer(tc_int_t value);
};

extern "C" {
Object *tc_integer_new(tc_int_t value);
}
