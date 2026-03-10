#include <cstring>

#include "TCString.h"

extern "C" {
Object *tc_string_new(const char *value) {
    TCString *str = new TCString{.m_Value = strdup(value)};

    return new Object{
            .m_Data = str,
            .m_Type = ObjectType::STRING,
            .m_Call = nullptr
    };
}

const char *tc_string_valueX(const Object *obj) {
    return static_cast<TCString *>(tinyclj_object_get_data(obj))->m_Value;
}
}
