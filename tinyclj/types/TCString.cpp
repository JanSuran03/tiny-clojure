#include <cstring>

#include "runtime/Runtime.h"
#include "TCString.h"

const Object *TCString::toString(const Object *self) {
    // strings are immutable, so we can just return self
    return self;
}

const Object *TCString::toEDN(const Object *self) {
    const char *value = static_cast<TCString *>(self->m_Data)->m_Value;
    std::string buf("\"");
    for (const char *c = value; *c; c++) {
        if (*c == '\n') {
            buf.append("\\n");
        } else if (*c == '\t') {
            buf.append("\\t");
        } else if (*c == '\r') {
            buf.append("\\r");
        } else if (*c == '\b') {
            buf.append("\\b");
        } else if (*c == '\"') {
            buf.append("\\\"");
        } else if (*c == '\\') {
            buf.append("\\\\");
        } else {
            buf.push_back(*c);
        }
    }
    return tc_string_new(buf.append("\"").c_str());
}

MethodTable TCString::st_MethodTable = MethodTable {
    .m_CallFn = nullptr,
    .m_ToStringFn = TCString::toString,
    .m_ToEdnFn = TCString::toEDN,
};

extern "C" {
Object *tc_string_new(const char *value) {
    TCString *str = new TCString{.m_Value = strdup(value)};

    return Runtime::getInstance().createObject(ObjectType::STRING, str, &TCString::st_MethodTable);
}
}
