#include "Object.h"
#include "compiler/CodegenContext.h"

Object Object::createStaticObject(ObjectType type, void *data, CallFn callFn) {
    return Object{
            .m_Data = data,
            .m_Type = type,
            .m_Call = callFn,
            .m_Static = true
    };
}

llvm::StructType *Object::getObjectStructType(CodegenContext &ctx) {
    static constexpr const char *structName = "Object";
    if (auto *ty = llvm::StructType::getTypeByName(*ctx.m_LLVMContext, structName)) {
        return ty;
    }
    return llvm::StructType::create(*ctx.m_LLVMContext,
                                    {ctx.pointerType(), // void *m_Data
                                     ctx.m_IRBuilder.getInt32Ty(), // ObjectType m_Type
                                     ctx.pointerType(), // CallFn m_Call
                                     ctx.m_IRBuilder.getInt8Ty(), // bool m_Marked
                                     ctx.m_IRBuilder.getInt8Ty() // bool m_Static
                                    },
                                    structName);
}

llvm::Value *Object::emitGetDataPtr(CodegenContext &ctx, llvm::Value *objPtr) {
    using namespace llvm;

    Value *dataFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getObjectStructType(ctx),
            objPtr,
            0, // index of m_Data field
            "data_field_ptr");
    // Load the void* value from the m_Data field
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), dataFieldPtr, "obj_data");
}

llvm::Value *Object::emitGetType(CodegenContext &ctx, llvm::Value *objPtr) {
    using namespace llvm;
    // Get the pointer to the m_Type field of the Object struct
    Value *typeFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getObjectStructType(ctx),
            objPtr,
            1, // index of m_Type field
            "type_field_ptr");
    // Load the ObjectType value from the m_Type field
    return ctx.m_IRBuilder.CreateLoad(ctx.m_IRBuilder.getInt32Ty(), typeFieldPtr, "obj_type");
}

llvm::Value *Object::emitGetCallFn(CodegenContext &ctx, llvm::Value *objPtr) {
    using namespace llvm;
    // Get the pointer to the m_Call field of the Object struct
    Value *callFnFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getObjectStructType(ctx),
            objPtr,
            2, // index of m_Call field
            "callfn_field_ptr");
    // Load the CallFn value from the m_Call field
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), callFnFieldPtr, "obj_callfn");
}
