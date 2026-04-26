#pragma once

#include "Object.h"

struct TCChar {
    char m_Value;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_char_new(char value);
}
