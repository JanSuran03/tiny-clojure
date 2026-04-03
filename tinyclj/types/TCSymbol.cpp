#include <cstring>

#include "runtime/Runtime.h"
#include "TCSymbol.h"

extern "C" {
Object *tc_symbol_new(const char *value) {
    TCSymbol *symbol = new TCSymbol{.m_Name = strdup(value)};

    return Runtime::getInstance().createObject(ObjectType::SYMBOL, symbol);
}

const char *tc_symbol_valueX(const Object *obj) {
    return static_cast<TCSymbol *>(obj->m_Data)->m_Name;
}
}