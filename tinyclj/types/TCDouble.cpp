#include "Runtime.h"
#include "TCDouble.h"

extern "C" {
Object *tc_double_new(tc_double_t value) {
    TCDouble *dbl = new TCDouble{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::DOUBLE, dbl);
}

tc_double_t tc_double_valueX(const Object *obj) {
    return static_cast<TCDouble *>(tinyclj_object_get_data(obj))->m_Value;
}
}