#pragma once

#include "Object.h"
#include "../tcdef.h"

struct TCBoolean {
    bool m_Value;
};

extern "C" {
Object *tc_boolean_new(bool value);

bool tc_boolean_valueX(const Object *obj);
}

Object *tc_boolean_new2(bool value);
