#include <cstring>

#include "runtime/Runtime.h"
#include "TCString.h"

extern "C" {
Object *tc_string_new(const char *value) {
    TCString *str = new TCString{.m_Value = strdup(value)};

    return Runtime::getInstance().createObject(ObjectType::STRING, str);
}
}
