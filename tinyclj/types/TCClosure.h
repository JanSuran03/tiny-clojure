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
Object *tc_closure_new(CallFn callStub, const Object **env, size_t numCaptures);

Object **tc_closure_allocate_env(size_t numCaptures);
}
