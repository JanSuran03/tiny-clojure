#pragma once

#include <atomic>
#include <unordered_set>
#include <vector>

#include "LoopBase.h"

struct CodegenContext {
    std::unique_ptr<llvm::LLVMContext> m_LLVMContext;
    llvm::IRBuilder<> m_IRBuilder;
    std::unique_ptr<llvm::Module> m_Module;
    std::vector<LoopBase> m_LoopLabels;
    llvm::Function *m_CurrentFunction = nullptr;
    std::unordered_map<std::string, llvm::AllocaInst *> m_VariableMap;
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

    llvm::IntegerType *getSizeType();

    void linkModule();
};