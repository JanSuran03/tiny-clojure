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

void CodegenContext::linkModule(const std::string &module_name) {
#if true
    static unsigned moduleCounter = 0;
    std::string ll_dst = std::string(PROJECT_SOURCE_DIR)
            .append("/modules/")
            .append("module_")
            .append(std::to_string(moduleCounter++));
    m_Module->setSourceFileName(module_name);
    // print to <ll_dst>.ll for debugging
    std::error_code ec;
    llvm::raw_fd_ostream dest(ll_dst + ".ll", ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Could not open file: " << ec.message();
    } else {
        llvm::errs() << "Dumping module to " << ll_dst + ".ll" << " for debugging\n";
        m_Module->print(dest, nullptr);
    }
#endif

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

llvm::Value *CodegenContext::registerGlobalString(const std::string &str) {
    using namespace llvm;
    GlobalVariable *globalStr;
    size_t strId;
    if (auto it = m_GlobalStringCache.find(str); it != m_GlobalStringCache.end()) {
        globalStr = it->second.second;
        strId = it->second.first;
    } else {
        strId = Runtime::getInstance().nextId();
        globalStr = new llvm::GlobalVariable(
                *m_Module,
                ArrayType::get(Type::getInt8Ty(*m_LLVMContext), str.size() + 1),
                true, // isConstant
                GlobalValue::PrivateLinkage,
                ConstantDataArray::getString(*m_LLVMContext, str),
                "str_" + std::to_string(strId)
        );
        m_GlobalStringCache.emplace(str, std::make_pair(strId, globalStr));
    }
    return m_IRBuilder.CreateBitCast(globalStr, Type::getInt8PtrTy(*m_LLVMContext), "strptr_" + std::to_string(strId));
}
