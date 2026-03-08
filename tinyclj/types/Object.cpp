#include "Object.h"

extern "C"
inline void *tinyclj_object_get_data(const Object *obj) {
    return obj->m_Data;
}

extern "C"
inline ObjectType tinyclj_object_get_type(const Object *obj) {
    return obj->m_Type;
}
