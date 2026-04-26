#include <cstring>

#include "runtime/Runtime.h"
#include "TCString.h"
#include "TCSymbol.h"

const Object *TCSymbol::toString(const Object *self) {
    return tc_string_new(static_cast<TCSymbol *>(self->m_Data)->m_Name);
}

const Object *TCSymbol::toEDN(const Object *self) {
    return tc_string_new(static_cast<TCSymbol *>(self->m_Data)->m_Name);
}

MethodTable TCSymbol::st_MethodTable = MethodTable {
    .m_CallFn = nullptr,
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