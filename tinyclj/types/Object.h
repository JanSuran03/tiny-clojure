#pragma once

#include <cstddef>

enum class ObjectType {
    BOOLEAN,
    INTEGER,
    DOUBLE,
    LIST,
    CHARACTER,
    STRING,
    SYMBOL,
    FUNCTION,
};

struct Object {
    void *m_Data;
    ObjectType m_Type;
    Object *(*m_Call)(Object *self, Object *args);
};

extern "C"
void *tinyclj_object_get_data(const Object *obj);

extern "C"
ObjectType tinyclj_object_get_type(const Object *obj);
