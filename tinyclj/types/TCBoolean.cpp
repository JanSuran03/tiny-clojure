#include "Runtime.h"
#include "TCBoolean.h"

extern "C" {
Object *tc_boolean_new(bool value) {
    TCBoolean *bool_data = new TCBoolean{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::BOOLEAN, bool_data);
}

bool tc_boolean_valueX(const Object *obj) {
    return static_cast<TCBoolean *>(tinyclj_object_get_data(obj))->m_Value;
}
}