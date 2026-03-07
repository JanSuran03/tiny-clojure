#include "DoubleExpr.h"

llvm::Value *DoubleExpr::getConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_double_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_double_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(ctx.m_LLVMContext), // return type: i8*
            {llvm::Type::getDoubleTy(ctx.m_LLVMContext)}, // parameter type: double
            false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_double_new", ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantFP::get(ctx.m_LLVMContext, llvm::APFloat(m_Value));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
    //return llvm::ConstantFP::get(ctx.m_LLVMContext, llvm::APFloat(m_Value));
}

DoubleExpr::DoubleExpr(double value) : m_Value(value) {}
