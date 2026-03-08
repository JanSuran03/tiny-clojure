#pragma once

#include "Object.h"
#include "../tcdef.h"

struct TCSymbol {
    char *m_Value;
};

extern "C" {
Object *tc_symbol_new(const char *value);

const char *tc_symbol_valueX(const Object *obj);
}
