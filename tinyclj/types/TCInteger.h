#pragma once

#include "Object.h"
#include "../tcdef.h"

struct TCInteger {
    tc_int_t m_Value;
};

extern "C" {
Object *tc_integer_new(tc_int_t value);

tc_int_t tc_integer_valueX(Object *obj);
}
