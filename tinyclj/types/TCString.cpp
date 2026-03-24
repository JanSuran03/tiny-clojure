#include <cstring>

#include "Runtime.h"
#include "TCString.h"

extern "C" {
Object *tc_string_new(const char *value) {
    TCString *str = new TCString{.m_Value = strdup(value)};

    return Runtime::getInstance().createObject(ObjectType::STRING, str);
}

const char *tc_string_valueX(const Object *obj) {
    return static_cast<TCString *>(tinyclj_object_get_data(obj))->m_Value;
}
}
