#pragma once

#include "Object.h"

struct TCChar {
    char m_Value;
};

extern "C" {
Object *tc_char_new(char value);

char tc_char_valueX(const Object *obj);
}
