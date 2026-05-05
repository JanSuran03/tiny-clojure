#include <sstream>

#include "runtime/Runtime.h"
#include "TCBoolean.h"
#include "TCDouble.h"
#include "TCString.h"

const Object *TCDouble::equals(const Object *self, const Object *other) {
    tc_double_t selfValue = static_cast<const TCDouble *>(self->m_Data)->m_Value;
    tc_double_t otherValue = static_cast<const TCDouble *>(other->m_Data)->m_Value;
    return TCBoolean::getStatic(selfValue == otherValue);
}

const Object *TCDouble::toString(const Object *self) {
    tc_double_t value = static_cast<TCDouble *>(self->m_Data)->m_Value;

    // cannot call std::to_string directly as it produces a lot of trailing zeros
    std::ostringstream oss;
    oss << std::defaultfloat << value;
    return tc_string_new(oss.str().c_str());
}

const Object *TCDouble::toEDN(const Object *self) {
    tc_double_t value = static_cast<TCDouble *>(self->m_Data)->m_Value;

    // EDN format for doubles is the same as the default string format
    std::ostringstream oss;
    oss << std::defaultfloat << value;
    return tc_string_new(oss.str().c_str());
}

MethodTable TCDouble::st_MethodTable = MethodTable{
        .m_CallFn = nullptr,
        .m_EqualsFn = TCDouble::equals,
        .m_ToStringFn = TCDouble::toString,
        .m_ToEdnFn = TCDouble::toEDN,
};

extern "C" {
Object *tc_double_new(tc_double_t value) {
    TCDouble *dbl = new TCDouble{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::DOUBLE, dbl, &TCDouble::st_MethodTable);
}
}