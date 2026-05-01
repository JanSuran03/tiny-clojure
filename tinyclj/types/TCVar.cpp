#include <cstring>
#include <stdexcept>

#include "TCString.h"
#include "TCVar.h"
#include "compiler/CodegenContext.h"
#include "runtime/Runtime.h"

const Object *tc_var_invoke(const Object *self, unsigned argc, const Object **argv) {
    const TCVar *var = static_cast<const TCVar *>(self->m_Data);
    const Object *root = var->m_Root;
    if (root == nullptr) {
        throw std::runtime_error("Cannot invoke unbound var");
    }
    CallFn callFn = root->m_MethodTable->m_CallFn;
    if (callFn == nullptr) {
        throw std::runtime_error("Cannot call nil");
    } else {
        return callFn(root, argc, argv);
    }
}

const Object *TCVar::toString(const Object *self) {
    const TCVar *var = static_cast<const TCVar *>(self->m_Data);
    return tc_string_new((std::string("(var") + var->m_Name + ")").c_str());
}

const Object *TCVar::toEDN(const Object *self) {
    const TCVar *var = static_cast<const TCVar *>(self->m_Data);
    return tc_string_new((std::string("(var") + var->m_Name + ")").c_str());
}

MethodTable TCVar::st_MethodTable = MethodTable{
        .m_CallFn = tc_var_invoke,
        .m_ToStringFn = TCVar::toString,
        .m_ToEdnFn = TCVar::toEDN,
};

llvm::StructType *TCVar::getVarStructType(CodegenContext &ctx) {
    static constexpr const char *structName = "TCVar";
    if (auto *ty = llvm::StructType::getTypeByName(*ctx.m_LLVMContext, structName)) {
        return ty;
    }
    return llvm::StructType::create(*ctx.m_LLVMContext,
                                    {ctx.pointerType(), // const Object* m_Root
                                     ctx.m_IRBuilder.getInt8PtrTy(), // char* m_Name
                                     ctx.m_IRBuilder.getInt8Ty() // bool m_IsMacro
                                    },
                                    structName);
}

llvm::Value *TCVar::emitGetRoot(CodegenContext &ctx, llvm::Value *varObjPtr) {
    using namespace llvm;
    // Get the pointer to the m_Data field of the Object struct
    Value *var_data_ptr = Object::emitGetDataPtr(ctx, varObjPtr);
    // Cast the void* data pointer to a TCVar* pointer
    Value *var_struct_ptr = ctx.m_IRBuilder.CreateBitCast(
            var_data_ptr,
            PointerType::get(getVarStructType(ctx), 0),
            "var_struct_ptr");
    // Get the pointer to the m_Root field of the TCVar struct
    Value *rootFieldPtr = ctx.m_IRBuilder.CreateStructGEP(
            getVarStructType(ctx),
            var_struct_ptr,
            0, // index of m_Root field
            "var_root_field_ptr");
    // Load the const Object* value from the m_Root field
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), rootFieldPtr, "var_root");
}

extern "C" {
Object *tc_var_new(const char *name) {
    TCVar *var = new TCVar{.m_Name = strdup(name)};

    return Runtime::getInstance().createObject(ObjectType::VAR, var, &TCVar::st_MethodTable);
}

const Object *tc_var_get_root(Object *var) {
    return static_cast<const TCVar *>(var->m_Data)->m_Root;
}

void tc_var_bind_root(Object *var, const Object *obj) {
    static_cast<TCVar *>(var->m_Data)->m_Root = obj;
}

bool tc_var_is_macroX(const Object *var) {
    return static_cast<const TCVar *>(var->m_Data)->m_IsMacro;
}

void tc_var_set_macroX(Object *var, bool is_macro) {
    static_cast<TCVar *>(var->m_Data)->m_IsMacro = is_macro;
}
}
