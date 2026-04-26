#include "runtime/Runtime.h"
#include "TCChar.h"
#include "TCString.h"

const Object *TCChar::toString(const Object *self) {
    char value = static_cast<TCChar *>(self->m_Data)->m_Value;
    char str[2] = {value, '\0'}; // create a null-terminated string
    return tc_string_new(str);
}

const Object *TCChar::toEDN(const Object *self) {
    char value = static_cast<TCChar *>(self->m_Data)->m_Value;
    switch (value) {
        case '\n':
            return tc_string_new("\\newline");
        case ' ':
            return tc_string_new("\\space");
        case '\t':
            return tc_string_new("\\tab");
        case '\r':
            return tc_string_new("\\return");
        case '\b':
            return tc_string_new("\\backspace");
        default: {
            char str[2] = {value, '\0'}; // create a null-terminated string
            return tc_string_new(str);
        }
    }
}

MethodTable TCChar::st_MethodTable = MethodTable{
        .m_CallFn = nullptr,
        .m_ToStringFn = TCChar::toString,
        .m_ToEdnFn = TCChar::toEDN,
};

extern "C" {
Object *tc_char_new(char value) {
    TCChar *char_data = new TCChar{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::CHARACTER, char_data, &TCChar::st_MethodTable);
}
}