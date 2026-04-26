#pragma once

#include "Object.h"

struct TCString {
    char *m_Value;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_string_new(const char *value);
}
