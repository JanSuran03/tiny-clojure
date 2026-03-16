#include "CompilerContext.h"

FunctionFrame::FunctionFrame(FunctionFrame *parent) : m_ParentFrame(parent) {}

CompilerContext::CompilerContext(Runtime &runtimeRef,
                                 llvm::LLVMContext &llvmContext,
                                 llvm::IRBuilder<> &irBuilder,
                                 llvm::Module &module,
                                 std::atomic<size_t> &idCounter)
        : m_RuntimeRef(runtimeRef),
          m_LLVMContext(llvmContext),
          m_IRBuilder(irBuilder),
          m_Module(module),
          m_IdCounter(idCounter) {
    declareStdLibFunctions();
}

void CompilerContext::declareStdLibFunctions() {
    // todo?
    //m_Module.getOrInsertFunction("tc_integer_valueX", llvm::Type::getInt64Ty(m_LLVMContext),
    //                             pointerType());
    //m_Module.getOrInsertFunction("tc_double_valueX", llvm::Type::getDoubleTy(m_LLVMContext),
    //                             pointerType());
    //m_Module.getOrInsertFunction("tinyclj_object_get_data", llvm::Type::getInt8PtrTy(m_LLVMContext),
    //                             pointerType());
    //m_Module.getOrInsertFunction("tinyclj_object_get_type", llvm::Type::getInt32Ty(m_LLVMContext),
    //                             pointerType());
    //m_Module.getOrInsertFunction("tinyclj_rt_add", objectPointerType(), pointerType());
}

llvm::PointerType *CompilerContext::pointerType() const {
    return llvm::PointerType::get(m_LLVMContext, 0);
}

llvm::PointerType *CompilerContext::pointerArrayType() const {
    return llvm::PointerType::get(pointerType(), 0);
}

void CompilerContext::newTmpBasicBlock() {
    auto block_id = "block__" + std::to_string(m_IdCounter++);
    llvm::BasicBlock *block = llvm::BasicBlock::Create(m_LLVMContext, block_id, m_CurrentFunction);
    m_IRBuilder.CreateBr(block);
    m_IRBuilder.SetInsertPoint(block);
}

size_t CompilerContext::nextId() {
    return m_IdCounter++;
}

llvm::BasicBlock *CompilerContext::createBasicBlock(const std::string &name) {
    return llvm::BasicBlock::Create(m_LLVMContext,
                                    name,
                                    m_CurrentFunction);
}
