#include "runtime/Runtime.h"
#include "TCBoolean.h"
#include "TCInteger.h"
#include "TCString.h"

const Object *TCInteger::equals(const Object *self, const Object *other) {
    tc_int_t selfValue = static_cast<const TCInteger *>(self->m_Data)->m_Value;
    tc_int_t otherValue = static_cast<const TCInteger *>(other->m_Data)->m_Value;
    return TCBoolean::getStatic(selfValue == otherValue);
}

const Object *TCInteger::toString(const Object *self) {
    tc_int_t value = static_cast<TCInteger *>(self->m_Data)->m_Value;
    return tc_string_new(std::to_string(value).c_str());
}

const Object *TCInteger::toEDN(const Object *self) {
    tc_int_t value = static_cast<TCInteger *>(self->m_Data)->m_Value;
    return tc_string_new(std::to_string(value).c_str());
}

MethodTable TCInteger::st_MethodTable = MethodTable {
    .m_CallFn = nullptr,
    .m_EqualsFn = TCInteger::equals,
    .m_ToStringFn = TCInteger::toString,
    .m_ToEdnFn = TCInteger::toEDN,
};

extern "C" {
Object *tc_integer_new(tc_int_t value) {
    TCInteger *int_data = new TCInteger{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::INTEGER, int_data, &TCInteger::st_MethodTable);
}
}
