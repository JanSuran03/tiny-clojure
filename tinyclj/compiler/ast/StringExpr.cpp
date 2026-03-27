#include "StringExpr.h"
#include "types/TCString.h"

llvm::Value *StringExpr::emitConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_string_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_string_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt8PtrTy(ctx.m_LLVMContext)}, // parameter type: char *
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_string_new", ctx.m_Module);
    }
    llvm::Value *strPtr = ctx.m_IRBuilder.CreateGlobalStringPtr(m_Value, "str");
    return ctx.m_IRBuilder.CreateCall(func, {strPtr}, "str_obj");
}

Object *StringExpr::evalConstantValue() const {
    return tc_string_new(m_Value.c_str());
}

StringExpr::StringExpr(std::string value) : m_Value(std::move(value)) {}
