#include "CompilerContext.h"

CompilerContext::CompilerContext(llvm::LLVMContext &llvmContext,
                                 llvm::IRBuilder<> &irBuilder,
                                 llvm::Module &module)
        : m_LLVMContext(llvmContext),
          m_IRBuilder(irBuilder),
          m_Module(module) {
    declareStdLibFunctions();
}

void CompilerContext::declareStdLibFunctions() {
    // todo?
    m_Module.getOrInsertFunction("tc_integer_valueX", llvm::Type::getInt64Ty(m_LLVMContext),
                                 objectPointerType());
    m_Module.getOrInsertFunction("tc_double_valueX", llvm::Type::getDoubleTy(m_LLVMContext),
                                 objectPointerType());
    m_Module.getOrInsertFunction("tinyclj_object_get_data", llvm::Type::getInt8PtrTy(m_LLVMContext),
                                 objectPointerType());
    m_Module.getOrInsertFunction("tinyclj_object_get_type", llvm::Type::getInt32Ty(m_LLVMContext),
                                 objectPointerType());
}

llvm::PointerType *CompilerContext::objectPointerType() const {
    return llvm::PointerType::get(m_LLVMContext, 0);
}
