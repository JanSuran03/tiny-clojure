#include <cstring>

#include "runtime/Runtime.h"
#include "TCBoolean.h"
#include "TCString.h"
#include "TCSymbol.h"

const Object *TCSymbol::equals(const Object *self, const Object *other) {
    if (self == other) {
        return tc_boolean_const_true;
    }

    if (other->m_Type != ObjectType::SYMBOL) {
        return tc_boolean_const_false;
    }

    const char *selfValue = static_cast<const TCSymbol *>(self->m_Data)->m_Name;
    const char *otherValue = static_cast<const TCSymbol *>(other->m_Data)->m_Name;
    return TCBoolean::getStatic(strcmp(selfValue, otherValue) == 0);
}

const Object *TCSymbol::toString(const Object *self) {
    return tc_string_new(static_cast<TCSymbol *>(self->m_Data)->m_Name);
}

const Object *TCSymbol::toEDN(const Object *self) {
    return tc_string_new(static_cast<TCSymbol *>(self->m_Data)->m_Name);
}

MethodTable TCSymbol::st_MethodTable = MethodTable {
    .m_CallFn = nullptr,
    .m_EqualsFn = TCSymbol::equals,
    .m_ToStringFn = TCSymbol::toString,
    .m_ToEdnFn = TCSymbol::toEDN,
};

extern "C" {
Object *tc_symbol_new(const char *value) {
    TCSymbol *symbol = new TCSymbol{.m_Name = strdup(value)};

    return Runtime::getInstance().createObject(ObjectType::SYMBOL, symbol, &TCSymbol::st_MethodTable);
}

const char *tc_symbol_valueX(const Object *obj) {
    return static_cast<TCSymbol *>(obj->m_Data)->m_Name;
}
}