#include "TCInteger.h"

extern "C" {
Object *tc_integer_new(tc_int_t value) {
    TCInteger *integer = new TCInteger{.m_Value = value};

    return new Object{
            .m_Data = integer,
            .m_Type = ObjectType::INTEGER,
            .m_Call = nullptr
    };
}

tc_int_t tc_integer_valueX(const Object *obj) {
    return static_cast<TCInteger *>(tinyclj_object_get_data(obj))->m_Value;
}
}
