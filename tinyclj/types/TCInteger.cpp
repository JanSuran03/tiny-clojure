#include "runtime/Runtime.h"
#include "TCInteger.h"

extern "C" {
Object *tc_integer_new(tc_int_t value) {
    TCInteger *int_data = new TCInteger{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::INTEGER, int_data);
}

tc_int_t tc_integer_valueX(const Object *obj) {
    return static_cast<TCInteger *>(tinyclj_object_get_data(obj))->m_Value;
}
}
