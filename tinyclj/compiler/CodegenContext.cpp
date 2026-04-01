#include "llvm/IR/Verifier.h"

#include "CodegenContext.h"
#include "runtime/Runtime.h"

CodegenContext::CodegenContext()
        : m_LLVMContext(std::make_unique<llvm::LLVMContext>()),
          m_IRBuilder(*m_LLVMContext),
          m_Module(std::make_unique<llvm::Module>("eval_module", *m_LLVMContext)) {}


llvm::PointerType *CodegenContext::pointerType() {
    return llvm::PointerType::get(*m_LLVMContext, 0);
}

llvm::PointerType *CodegenContext::pointerArrayType() {
    return llvm::PointerType::get(pointerType(), 0);
}

void CodegenContext::jumpToTmpBasicBlock() {
    auto block_id = "block__" + std::to_string(Runtime::getInstance().nextId());
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_LLVMContext, block_id, m_CurrentFunction);
    m_IRBuilder.CreateBr(block);
    m_IRBuilder.SetInsertPoint(block);
}

llvm::BasicBlock *CodegenContext::createBasicBlock(const std::string &name) {
    return llvm::BasicBlock::Create(*m_LLVMContext,
                                    name,
                                    m_CurrentFunction);
}

bool CodegenContext::currentBlockTerminated() const {
    return m_IRBuilder.GetInsertBlock()->getTerminator() != nullptr;
}

void CodegenContext::linkModule() {
    if (llvm::verifyModule(*m_Module, &llvm::errs())) {
        m_Module->dump();
        throw std::runtime_error("Module verification failed");
    }

    llvm::orc::ThreadSafeModule tsm(std::move(m_Module), std::move(m_LLVMContext));

    if (auto err = Runtime::getInstance().getJIT()->addIRModule(std::move(tsm))) {
        throw std::runtime_error("Failed to add module to JIT: " + llvm::toString(std::move(err)));
    }
}

std::vector<llvm::AllocaInst *> &CodegenContext::currentInvokeArgvAllocas() {
    return m_InvokeArgvAllocasStack.back();
}
