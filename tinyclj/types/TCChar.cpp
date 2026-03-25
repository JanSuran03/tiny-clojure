#include "runtime/Runtime.h"
#include "TCChar.h"

extern "C" {
Object *tc_char_new(char value) {
    TCChar *char_data = new TCChar{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::CHARACTER, char_data);
}

char tc_char_valueX(const Object *obj) {
    return static_cast<TCChar *>(tinyclj_object_get_data(obj))->m_Value;
}
}