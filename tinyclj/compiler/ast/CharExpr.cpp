#include "CharExpr.h"
#include "types/TCChar.h"

EmitResult CharExpr::emitIR(CodegenContext &ctx) const {
    // call a linked function "tc_char_new" with m_Value
    llvm::Function *func = ctx.m_Module->getFunction("tc_char_new");
    if (!func) {
        // declare the function
        llvm::FunctionType *funcType = llvm::FunctionType::get(
                ctx.pointerType(), // return type: Object*
                {llvm::Type::getInt8Ty(*ctx.m_LLVMContext)}, // parameter type: i8 (char)
                false // isVarArg
        );
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "tc_char_new", *ctx.m_Module);
    }
    llvm::Value *arg = llvm::ConstantInt::get(*ctx.m_LLVMContext, llvm::APInt(8, m_Value, true));
    return ctx.m_IRBuilder.CreateCall(func, {arg});
}

Object *CharExpr::eval() const {
    return tc_char_new(m_Value);
}

CharExpr::CharExpr(char value) : m_Value(value) {}
