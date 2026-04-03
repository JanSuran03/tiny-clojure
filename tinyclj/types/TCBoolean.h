#pragma once

#include "Object.h"

struct TCBoolean {
    bool m_Value;

    static llvm::StructType *getBooleanStructType(CodegenContext &ctx);

    static llvm::Value *emitGetValue(CodegenContext &ctx, llvm::Value *boolDataPtr);
};

extern "C" {
Object *tc_boolean_new(bool value);
}
