#include "StringExpr.h"

llvm::Value *StringExpr::getConstantValue(CompilerContext &ctx) const {
    // Create a global string and reference it here
    /*llvm::Constant *strConstant = llvm::ConstantDataArray::getString(ctx.m_LLVMContext, m_Value, true);
    llvm::GlobalVariable *strVar = new llvm::GlobalVariable(
            ctx.m_Module,
            strConstant->getType(),
            true, // isConstant
            llvm::GlobalValue::PrivateLinkage,
            strConstant
    );

    return llvm::ConstantExpr::getGetElementPtr(
            llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx.m_LLVMContext), m_Value.size() + 1),
            strVar,
            {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 0),
             llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.m_LLVMContext), 0)}
    );*/
    throw std::runtime_error("String constants not implemented yet");
}

StringExpr::StringExpr(std::string value) : m_Value(std::move(value)) {}
