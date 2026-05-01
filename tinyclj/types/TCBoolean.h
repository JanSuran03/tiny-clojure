#pragma once

#include "compiler/CodegenContext.h"
#include "llvm/IR/IRBuilder.h"

#include "Object.h"

extern "C" {
extern Object *tc_boolean_const_true;
extern Object *tc_boolean_const_false;
}

struct TCBoolean {
    bool m_Value;

    static llvm::StructType *getBooleanStructType(CodegenContext &ctx);

    static llvm::Value *emitGetValue(CodegenContext &ctx, llvm::Value *boolDataPtr);

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;

    static inline Object *getStatic(bool value) {
        return value ? tc_boolean_const_true : tc_boolean_const_false;
    }

    static void init();
};
