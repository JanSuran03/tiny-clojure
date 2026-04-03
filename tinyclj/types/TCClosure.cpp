#include "TCClosure.h"
#include "compiler/CodegenContext.h"
#include "runtime/Runtime.h"

llvm::StructType *TCClosure::getClosureStructType(CodegenContext &ctx) {
    static constexpr const char *structName = "TCClosure";
    if (auto *ty = llvm::StructType::getTypeByName(*ctx.m_LLVMContext, structName)) {
        return ty;
    }
    return llvm::StructType::create(
            *ctx.m_LLVMContext,
            {ctx.pointerType(), // Object** m_Env
             ctx.m_IRBuilder.getInt64Ty()}, // size_t m_NumCaptures
            structName);
}

llvm::Value *TCClosure::emitGetEnv(CodegenContext &ctx, llvm::Value *closureObjPtr) {
    using namespace llvm;
    // Get the pointer to the m_Data field of the Object struct
    Value *closure_data_ptr = Object::emitGetDataPtr(ctx, closureObjPtr);
    // Cast the void* data pointer to a TCClosure* pointer
    Value *closure_struct_ptr = ctx.m_IRBuilder.CreateBitCast(
            closure_data_ptr,
            PointerType::get(getClosureStructType(ctx), 0),
            "closure_struct_ptr");
    // Get the pointer to the m_Env field of the TCClosure struct
    Value *envFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getClosureStructType(ctx),
            closure_struct_ptr,
            0, // index of m_Env field
            "closure_env_field_ptr");
    // Load the Object** value from the m_Env field
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), envFieldPtr, "closure_env");
}

extern "C" {
Object *tc_closure_new(CallFn callStub, const Object **env, size_t numCaptures) {
    TCClosure *closure = new TCClosure{.m_Env = env, .m_NumCaptures = numCaptures};

    return Runtime::getInstance().createObject(ObjectType::CLOSURE, closure, callStub);
}

Object **tc_closure_allocate_env(size_t numCaptures) {
    return new Object *[numCaptures];
}
}
