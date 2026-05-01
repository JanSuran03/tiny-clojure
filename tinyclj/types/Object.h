#pragma once

#include <cstddef>
#include <optional>

#include "llvm/IR/IRBuilder.h"

class CodegenContext;

enum class ObjectType {
    BOOLEAN,
    CHARACTER,
    CLOSURE,
    DOUBLE,
    FUNCTION,
    INTEGER,
    LIST,
    STRING,
    SYMBOL,
    VAR
};

struct Object;
struct TCString;

using CallFn = const Object *(*)(const Object *self,
                                 unsigned argc,
                                 const struct Object **argv);

using UnaryFn = const Object *(*)(const Object *self);

const Object *tc_object_to_string(const Object *obj);

const Object *tc_object_to_edn(const Object *obj);

struct MethodTable {
    CallFn m_CallFn;
    UnaryFn m_ToStringFn;
    UnaryFn m_ToEdnFn;
};

struct Object {
    void *m_Data;
    ObjectType m_Type;
    const MethodTable *m_MethodTable;

    // GC: mark & sweep
    mutable bool m_Marked = false;
    bool m_Static = false; // don't destroy static objects: empty_list etc.

    static Object createStaticObject(ObjectType type, void *data, const MethodTable *methodTable);

    static llvm::StructType *getObjectStructType(CodegenContext &ctx);

    static llvm::Value *emitGetDataPtr(CodegenContext &ctx, llvm::Value *objPtr);

    static llvm::Value *emitGetType(CodegenContext &ctx, llvm::Value *objPtr);

    static llvm::Value *emitGetCallFn(CodegenContext &ctx, llvm::Value *objPtr);

    static llvm::StructType *getMethodTableStructType(llvm::LLVMContext &llvmContext);
};
