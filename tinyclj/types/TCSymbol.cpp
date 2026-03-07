#include "TCSymbol.h"
#include <cstring>

extern "C" {
Object *tc_symbol_new(const char *value) {
    TCSymbol *symbol = new TCSymbol{.m_Value = strdup(value)};

    return new Object{
            .m_Data = symbol,
            .m_Type = ObjectType::SYMBOL,
            .m_Call = nullptr
    };
}

const char *tc_symbol_name(Object *obj) {
    return static_cast<TCSymbol *>(tinyclj_object_get_data(obj))->m_Value;
}
}