#include "TCDouble.h"

extern "C" {
Object *tc_double_new(tc_double_t value) {
    TCDouble *dbl = new TCDouble{.m_Value = value};

    return new Object{
            .m_Data = dbl,
            .m_Type = ObjectType::DOUBLE,
            .m_Call = nullptr
    };
}

tc_double_t tc_double_valueX(Object *obj) {
    return static_cast<TCDouble *>(tinyclj_object_get_data(obj))->m_Value;
}
}