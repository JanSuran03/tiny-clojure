#pragma once

#include <atomic>
#include <unordered_set>
#include <vector>

#include "RecursionPoint.h"

struct CodegenContext {
    std::unique_ptr<llvm::LLVMContext> m_LLVMContext;
    llvm::IRBuilder<> m_IRBuilder;
    std::unique_ptr<llvm::Module> m_Module;
    std::vector<RecursionPoint> m_LoopLabels;
    llvm::Function *m_CurrentFunction = nullptr;
    /// Argv allocas for each invoke expression, for each function frame
    std::vector<std::vector<llvm::AllocaInst *>> m_InvokeArgvAllocasStack;
    std::unordered_map<std::string, llvm::Value *> m_VariableMap;
    llvm::Argument *m_ClosureEnv = nullptr;

    llvm::PointerType *pointerType();

    llvm::PointerType *pointerArrayType();

    // Creates a new basic block and a branch instruction to that block from the current block
    // and sets the IR insertion point to that block.
    void jumpToTmpBasicBlock();

    // Creates a basic block
    llvm::BasicBlock *createBasicBlock(const std::string &name);

    bool currentBlockTerminated() const;

    CodegenContext();

    void linkModule();

    std::vector<llvm::AllocaInst *> &currentInvokeArgvAllocas();
};