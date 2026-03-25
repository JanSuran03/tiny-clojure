#include <cstring>

#include "runtime/Runtime.h"
#include "TCFunction.h"

extern "C" {
Object *tc_function_new(CallFn callStub, const char *name) {
    TCFunction *function = new TCFunction{.m_Name = strdup(name)};

    return Runtime::getInstance().createObject(ObjectType::FUNCTION, function, callStub);
}
}