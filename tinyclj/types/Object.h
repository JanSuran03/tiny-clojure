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
    VAR
};

struct Object;

typedef Object *(*CallFn)(const Object *self,
                          size_t argc,
                          const struct Object **argv);

struct Object {
    void *m_Data;
    ObjectType m_Type;
    CallFn m_Call;

    // GC: mark & sweep
    mutable bool m_Marked = false;
    bool m_Static = false; // don't destroy static objects: empty_list etc.

    static Object createStaticObject(ObjectType type, void *data, CallFn callFn = nullptr);
};

extern "C" {
void *tinyclj_object_get_data(const Object *obj);

ObjectType tinyclj_object_get_type(const Object *obj);

CallFn tinyclj_object_get_callfn(const Object *obj);
}
