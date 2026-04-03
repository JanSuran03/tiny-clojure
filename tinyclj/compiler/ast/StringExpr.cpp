#include "StringExpr.h"
#include "types/TCString.h"

EmitResult StringExpr::emitIR(CodegenContext &ctx) const {
    // call a linked function "tc_string_new" with m_Name
    llvm::Function *func = ctx.m_Module->getFunction("tc_string_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt8PtrTy(*ctx.m_LLVMContext)}, // parameter type: char *
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_string_new", *ctx.m_Module);
    }

    llvm::Value *strPtr = ctx.registerGlobalString(m_Value);
    return ctx.m_IRBuilder.CreateCall(func, {strPtr}, "str_obj");
}

Object *StringExpr::eval() const {
    return tc_string_new(m_Value.c_str());
}

StringExpr::StringExpr(std::string value) : m_Value(std::move(value)) {}
