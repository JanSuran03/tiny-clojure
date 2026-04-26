#include "Object.h"
#include "TCString.h"
#include "compiler/CodegenContext.h"

const Object *tc_object_to_string(const Object *obj) {
    if (obj == nullptr) {
        return tc_string_new("");
    }
    UnaryFn toStringFn = obj->m_MethodTable->m_ToStringFn;
    if (toStringFn) {
        return toStringFn(obj);
    } else {
        // default toString implementation if no method is provided: print type and pointer value
        return tc_string_new(("<object of type " + std::to_string(static_cast<int>(obj->m_Type))
                              + " at " + std::to_string((uintptr_t) obj) + ">").c_str());
    }
}

const Object *tc_object_to_edn(const Object *obj) {
    if (obj == nullptr) {
        return tc_string_new("nil");
    }
    UnaryFn toEdnFn = obj->m_MethodTable->m_ToEdnFn;
    if (toEdnFn) {
        return toEdnFn(obj);
    } else {
        // default toEDN implementation if no method is provided: same as toString
        return tc_object_to_string(obj);
    }
}

Object Object::createStaticObject(ObjectType type, void *data, const MethodTable *methodTable) {
    return Object{
            .m_Data = data,
            .m_Type = type,
            .m_MethodTable = methodTable,
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
    Value *methodTableFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getObjectStructType(ctx),
            objPtr,
            2, // index of the m_MethodTable field
            "callfn_field_ptr");
    Value *methodTable = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), methodTableFieldPtr, "obj_methodTable");
    llvm::StructType *methodTableStructType = getMethodTableStructType(*ctx.m_LLVMContext);
    Value *callFnFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            methodTableStructType,
            methodTable,
            0, // index of m_CallFn field in MethodTable
            "callfn_ptr");
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), callFnFieldPtr, "obj_callFn");
}

llvm::StructType *Object::getMethodTableStructType(llvm::LLVMContext &llvmContext) {
    static constexpr const char *structName = "MethodTable";
    if (auto *ty = llvm::StructType::getTypeByName(llvmContext, structName)) {
        return ty;
    }
    return llvm::StructType::create(
            llvmContext,
            {llvm::PointerType::get(llvmContext, 0), // m_CallFn
             llvm::PointerType::get(llvmContext, 0), // m_ToStringFn
             llvm::PointerType::get(llvmContext, 0)}, // m_ToEdnFn
            structName);
}
