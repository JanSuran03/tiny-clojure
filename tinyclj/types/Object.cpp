#include "Object.h"

extern "C"
void *tinyclj_object_get_data(const Object *obj) {
    return obj->m_Data;
}

extern "C"
ObjectType tinyclj_object_get_type(const Object *obj) {
    return obj->m_Type;
}
