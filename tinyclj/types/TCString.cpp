#include <cstring>

#include "TCString.h"

extern "C" {
Object *tc_string_new(const char *value) {
    TCString *symbol = new TCString{.m_Value = strdup(value)};

    return new Object{
            .m_Data = symbol,
            .m_Type = ObjectType::SYMBOL,
            .m_Call = nullptr
    };
}

const char *tc_string_valueX(const Object *obj) {
    return static_cast<TCString *>(tinyclj_object_get_data(obj))->m_Value;
}
}
