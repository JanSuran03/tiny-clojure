#include "TCChar.h"

extern "C" {
Object *tc_char_new(char value) {
    TCChar *integer = new TCChar{.m_Value = value};

    return new Object{
            .m_Data = integer,
            .m_Type = ObjectType::CHARACTER,
            .m_Call = nullptr
    };
}

char tc_char_valueX(const Object *obj) {
    return static_cast<TCChar *>(tinyclj_object_get_data(obj))->m_Value;
}
}