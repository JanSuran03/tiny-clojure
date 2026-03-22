#include "DoubleExpr.h"
#include "types/TCDouble.h"

llvm::Value *DoubleExpr::emitConstantValue(CompilerContext &ctx) const {
    // call a linked function "tc_double_new" with m_Value
    llvm::Function *func = ctx.m_Module.getFunction("tc_double_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getDoubleTy(ctx.m_LLVMContext)}, // parameter type: double
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_double_new", ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantFP::get(ctx.m_LLVMContext, llvm::APFloat(m_Value));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
}

Object *DoubleExpr::evalConstantValue() const {
    return tc_double_new(m_Value);
}

DoubleExpr::DoubleExpr(tc_double_t value) : m_Value(value) {}
