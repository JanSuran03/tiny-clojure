#include "runtime/Runtime.h"
#include "TCDouble.h"

extern "C" {
Object *tc_double_new(tc_double_t value) {
    TCDouble *dbl = new TCDouble{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::DOUBLE, dbl);
}
}