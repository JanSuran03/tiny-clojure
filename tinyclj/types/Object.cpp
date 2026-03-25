#include "Object.h"

Object Object::createStaticObject(ObjectType type, void *data, CallFn callFn) {
    return Object{
            .m_Data = data,
            .m_Type = type,
            .m_Call = callFn,
            .m_Static = true
    };
}

extern "C" {
void *tinyclj_object_get_data(const Object *obj) {
    return obj->m_Data;
}
ObjectType tinyclj_object_get_type(const Object *obj) {
    return obj->m_Type;
}

CallFn tinyclj_object_get_callfn(const Object *obj) {
    return obj->m_Call;
}
}
