#include <iostream>

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

tc_int_t TCInteger::st_PreallocatedMin = -128;
tc_int_t TCInteger::st_PreallocatedMax = 127;
bool TCInteger::st_PreallocationEnabled = true;
Object *TCInteger::st_PreallocatedIntegers = nullptr;

Object *TCInteger::allocate_integer(tc_int_t value) {
    TCInteger *int_data = new TCInteger{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::INTEGER, int_data, &TCInteger::st_MethodTable);
}

void TCInteger::init() {
    if (!st_PreallocationEnabled) {
        return;
    } else if (st_PreallocatedMax < st_PreallocatedMin) {

        std::cerr << "Warning: Invalid integer cache range: [" << st_PreallocatedMin << ", " << st_PreallocatedMax
                  << "]. Preallocation disabled." << std::endl;
        return;
    }

    st_PreallocatedIntegers = new Object[st_PreallocatedMax - st_PreallocatedMin + 1];

    for (tc_int_t i = st_PreallocatedMin; i <= st_PreallocatedMax; i++) {
        TCInteger *int_data = new TCInteger{.m_Value = i};
        st_PreallocatedIntegers[i - st_PreallocatedMin] =
                Object::createStaticObject(ObjectType::INTEGER, int_data, &TCInteger::st_MethodTable);
    }
}

extern "C" {
Object *tc_integer_new(tc_int_t value) {
    if (TCInteger::st_PreallocationEnabled
        && value >= TCInteger::st_PreallocatedMin
        && value <= TCInteger::st_PreallocatedMax) {
        return &TCInteger::st_PreallocatedIntegers[value - TCInteger::st_PreallocatedMin];
    } else {
        return TCInteger::allocate_integer(value);
    }
}
}
