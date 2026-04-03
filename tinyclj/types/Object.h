#pragma once

#include <cstddef>
#include <optional>

#include "llvm/IR/IRBuilder.h"

class CodegenContext;

enum class ObjectType {
    BOOLEAN,
    INTEGER,
    DOUBLE,
    LIST,
    CHARACTER,
    STRING,
    SYMBOL,
    FUNCTION,
    CLOSURE,
    VAR
};

struct Object;

using CallFn = Object *(*)(const Object *self,
                           size_t argc,
                           const struct Object **argv);

struct Object {
    void *m_Data;
    ObjectType m_Type;
    CallFn m_Call;

    // GC: mark & sweep
    mutable bool m_Marked = false;
    bool m_Static = false; // don't destroy static objects: empty_list etc.

    static Object createStaticObject(ObjectType type, void *data, CallFn callFn = nullptr);

    static llvm::StructType *getObjectStructType(CodegenContext &ctx);

    static llvm::Value *emitGetDataPtr(CodegenContext &ctx, llvm::Value *objPtr);

    static llvm::Value *emitGetType(CodegenContext &ctx, llvm::Value *objPtr);

    static llvm::Value *emitGetCallFn(CodegenContext &ctx, llvm::Value *objPtr);
};
