#include <cstring>

#include "runtime/Runtime.h"
#include "TCFunction.h"
#include "TCString.h"

const Object *TCFunction::toString(const Object *self) {
    const TCFunction *function = static_cast<const TCFunction *>(self->m_Data);
    return tc_string_new((std::string("#<function:") + function->m_Name + ">").c_str());
}

extern "C" {
void tc_function_init_vtable(MethodTable *methodTable, CallFn callStub) {
    methodTable->m_CallFn = callStub;
    methodTable->m_ToStringFn = TCFunction::toString;
    methodTable->m_ToEdnFn = nullptr; // functions don't have a specific EDN representation
}

Object *tc_function_new(const MethodTable *methodTable, const char *name) {
    TCFunction *function = new TCFunction{.m_Name = strdup(name)};

    return Runtime::getInstance().createObject(ObjectType::FUNCTION, function, methodTable);
}
}