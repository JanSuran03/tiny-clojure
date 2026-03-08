#include "IntegerExpr.h"

llvm::Value *IntegerExpr::getConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_integer_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_integer_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(ctx.m_LLVMContext), // return type: i8*
            {llvm::Type::getInt64Ty(ctx.m_LLVMContext)}, // parameter type: i64
            false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_integer_new", ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantInt::get(ctx.m_LLVMContext, llvm::APInt(64, m_Value, true));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
    //return llvm::ConstantInt::get(ctx.m_LLVMContext, llvm::APInt(64, m_Value, true));
}

IntegerExpr::IntegerExpr(tc_int_t value) : m_Value(value) {}
