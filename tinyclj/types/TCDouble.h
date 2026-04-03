#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCDouble {
    tc_double_t m_Value;
};

extern "C" {
Object *tc_double_new(tc_double_t value);
}
