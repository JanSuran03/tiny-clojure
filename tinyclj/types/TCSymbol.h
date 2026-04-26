#pragma once

#include "Object.h"

struct TCSymbol {
    char *m_Name;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_symbol_new(const char *value);

const char *tc_symbol_valueX(const Object *obj);
}
