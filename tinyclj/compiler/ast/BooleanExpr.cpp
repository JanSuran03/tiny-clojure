#include "BooleanExpr.h"
#include "types/TCBoolean.h"

llvm::Value *BooleanExpr::emitConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_boolean_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_boolean_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt8Ty(ctx.m_LLVMContext)}, // parameter type: 8-bit integer
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_boolean_new", ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantInt::get(ctx.m_LLVMContext, llvm::APInt(8, m_Value ? 1 : 0, true));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
    //return llvm::ConstantInt::get(ctx.m_LLVMContext, llvm::APInt(64, m_Value, true));
}

Object *BooleanExpr::evalConstantValue() const {
    return tc_boolean_new(m_Value);
}

BooleanExpr::BooleanExpr(bool value) : m_Value(value) {}
