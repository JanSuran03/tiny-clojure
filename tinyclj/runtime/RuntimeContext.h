#pragma once

#include <atomic>
#include <unordered_set>

#include "LoopBase.h"

/**
 * Holds the internal state of the compiler.
 */
class CompilerContext {
    void declareStdLibFunctions();

public:
    llvm::LLVMContext &m_LLVMContext;
    llvm::IRBuilder<> &m_IRBuilder;
    llvm::Module &m_Module;
    std::vector<LoopBase> m_LoopLabels;
    llvm::Function *m_CurrentFunction = nullptr;
    std::atomic<int64_t> &m_IdCounter;
    std::unordered_map<std::string, llvm::AllocaInst *> m_VariableMap;
    std::unordered_set<std::string> m_AvailableSymbols = {"+"};

    CompilerContext(llvm::LLVMContext &llvmContext,
                    llvm::IRBuilder<> &irBuilder,
                    llvm::Module &module,
                    std::atomic<int64_t> &idCounter);

    llvm::PointerType *pointerType() const;

    // Creates a new basic block and a branch instruction to that block from the current block
    // and sets the IR insertion point to that block.
    void newBasicBlock();
};
