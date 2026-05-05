#include "Object.h"
#include "TCBoolean.h"
#include "TCInteger.h"
#include "TCString.h"
#include "runtime/Runtime.h"

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

MethodTable TCInteger::st_MethodTable = MethodTable{
        .m_CallFn = nullptr,
        .m_EqualsFn = TCInteger::equals,
        .m_ToStringFn = TCInteger::toString,
        .m_ToEdnFn = TCInteger::toEDN,
};

Object TCInteger::st_PreallocatedIntegers[PREALLOCATED_MAX - PREALLOCATED_MIN + 1];

Object *TCInteger::allocate_integer(tc_int_t value) {
    TCInteger *int_data = new TCInteger{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::INTEGER, int_data, &TCInteger::st_MethodTable);
}

void TCInteger::init() {
    for (tc_int_t i = PREALLOCATED_MIN; i <= PREALLOCATED_MAX; i++) {
        TCInteger *int_data = new TCInteger{.m_Value = i};
        st_PreallocatedIntegers[i - PREALLOCATED_MIN] =
                Object::createStaticObject(ObjectType::INTEGER, int_data, &TCInteger::st_MethodTable);
    }
}

extern "C" {
Object *tc_integer_new(tc_int_t value) {
    if (value >= TCInteger::PREALLOCATED_MIN && value <= TCInteger::PREALLOCATED_MAX) {
        return &TCInteger::st_PreallocatedIntegers[value - TCInteger::PREALLOCATED_MIN];
    } else {
        return TCInteger::allocate_integer(value);
    }
}
}
