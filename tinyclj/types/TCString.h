#pragma once

#include "Object.h"

struct TCString {
    char *m_Value;
};

extern "C" {
Object *tc_string_new(const char *value);
}
