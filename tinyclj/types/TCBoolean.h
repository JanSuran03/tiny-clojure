#pragma once

#include "Object.h"

struct TCBoolean {
    bool m_Value;

    static llvm::StructType *getBooleanStructType(CodegenContext &ctx);

    static llvm::Value *emitGetValue(CodegenContext &ctx, llvm::Value *boolDataPtr);

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
Object *tc_boolean_new(bool value);
}
