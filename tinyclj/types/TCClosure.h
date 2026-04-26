#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCClosure {
    const Object **m_Env;
    size_t m_NumCaptures;

    static llvm::StructType *getClosureStructType(CodegenContext &ctx);

    static llvm::Value *emitGetEnv(CodegenContext &ctx, llvm::Value *closureObjPtr);
};

extern "C" {
void tc_closure_init_vtable(MethodTable *methodTable, CallFn callStub);

Object *tc_closure_new(const MethodTable *methodTable, const Object **env, size_t numCaptures);

Object **tc_closure_allocate_env(size_t numCaptures);
}
