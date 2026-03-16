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
    CLOSURE,
};

struct Object;

typedef Object *(*CallFn)(const Object *self,
                          size_t argc,
                          const struct Object **argv);

struct Object {
    void *m_Data;
    ObjectType m_Type;

    CallFn m_Call;
};

extern "C" {
void *tinyclj_object_get_data(const Object *obj);

ObjectType tinyclj_object_get_type(const Object *obj);

CallFn tinyclj_object_get_callfn(const Object *obj);
}
