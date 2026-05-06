#include "TCBoolean.h"
#include "TCString.h"
#include "runtime/Runtime.h"

const Object *TCBoolean::equals(const Object *self, const Object *other) {
    return getStatic(self == other);
}

const Object *TCBoolean::toString(const Object *self) {
    bool value = static_cast<TCBoolean *>(self->m_Data)->m_Value;
    return tc_string_new(value ? "true" : "false");
}

const Object *TCBoolean::toEDN(const Object *self) {
    bool value = static_cast<TCBoolean *>(self->m_Data)->m_Value;
    return tc_string_new(value ? "true" : "false");
}

MethodTable TCBoolean::st_MethodTable = MethodTable{
        .m_CallFn = nullptr,
        .m_EqualsFn = TCBoolean::equals,
        .m_ToStringFn = TCBoolean::toString,
        .m_ToEdnFn = TCBoolean::toEDN,
};

Object *tc_boolean_create_static(bool value) {
    TCBoolean *bool_data = new TCBoolean{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::BOOLEAN, bool_data, &TCBoolean::st_MethodTable, true);
}

void TCBoolean::init() {
    tc_boolean_const_true = tc_boolean_create_static(true);
    tc_boolean_const_false = tc_boolean_create_static(false);
}

extern "C" {
Object *tc_boolean_const_true = nullptr;
Object *tc_boolean_const_false = nullptr;
}
