#include "IntegerExpr.h"
#include "types/TCInteger.h"

llvm::Value *IntegerExpr::emitConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_integer_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_integer_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt64Ty(ctx.m_LLVMContext)}, // parameter type: 64-bit integer
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_integer_new", ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantInt::get(ctx.m_LLVMContext, llvm::APInt(64, m_Value, true));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
}

Object *IntegerExpr::evalConstantValue() const {
    return tc_integer_new(m_Value);
}

IntegerExpr::IntegerExpr(tc_int_t value) : m_Value(value) {}
