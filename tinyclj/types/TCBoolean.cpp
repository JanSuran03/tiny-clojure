#include "TCBoolean.h"
#include "TCString.h"
#include "compiler/CodegenContext.h"
#include "runtime/Runtime.h"

llvm::StructType *TCBoolean::getBooleanStructType(CodegenContext &ctx) {
    static constexpr const char *structName = "TCBoolean";
    if (auto *ty = llvm::StructType::getTypeByName(*ctx.m_LLVMContext, structName)) {
        return ty;
    }
    return llvm::StructType::create(*ctx.m_LLVMContext,
                                    {ctx.m_IRBuilder.getInt8Ty()}, // bool m_Value
                                    structName);
}

llvm::Value *TCBoolean::emitGetValue(CodegenContext &ctx, llvm::Value *boolDataPtr) {
    using namespace llvm;
    // Get the pointer to the m_Data field of the Object struct
    Value *bool_data_ptr = Object::emitGetDataPtr(ctx, boolDataPtr);
    // Cast the void* data pointer to a TCBoolean* pointer
    Value *bool_struct_ptr = ctx.m_IRBuilder.CreateBitCast(
            bool_data_ptr,
            PointerType::get(getBooleanStructType(ctx), 0),
            "bool_struct_ptr");
    // Get the pointer to the m_Value field of the TCBoolean struct
    Value *valueFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getBooleanStructType(ctx),
            bool_struct_ptr,
            0, // index of m_Value field
            "bool_value_field_ptr");
    // Load the i8 value from the m_Value field and return it
    return ctx.m_IRBuilder.CreateLoad(ctx.m_IRBuilder.getInt8Ty(), valueFieldPtr, "bool_value");
}

const Object *TCBoolean::toString(const Object *self) {
    bool value = static_cast<TCBoolean *>(self->m_Data)->m_Value;
    return tc_string_new(value ? "true" : "false");
}

const Object *TCBoolean::toEDN(const Object *self) {
    bool value = static_cast<TCBoolean *>(self->m_Data)->m_Value;
    return tc_string_new(value ? "true" : "false");
}

MethodTable TCBoolean::st_MethodTable = MethodTable{
        .m_CallFn = nullptr,
        .m_ToStringFn = TCBoolean::toString,
        .m_ToEdnFn = TCBoolean::toEDN,
};

Object *tc_boolean_create_static(bool value) {
    TCBoolean *bool_data = new TCBoolean{.m_Value = value};

    return Runtime::getInstance().createObject(ObjectType::BOOLEAN, bool_data, &TCBoolean::st_MethodTable, true);
}

void TCBoolean::init() {
    tc_boolean_const_true = tc_boolean_create_static(true);
    tc_boolean_const_false = tc_boolean_create_static(false);
}

extern "C" {
Object *tc_boolean_const_true = nullptr;
Object *tc_boolean_const_false = nullptr;
}
