#pragma once

#include <atomic>
#include <unordered_set>

#include "LoopBase.h"

class Runtime;

struct FunctionFrame {
    std::unordered_map<std::string, int> m_Captures;
    std::unordered_set<std::string> m_Locals;
    FunctionFrame *m_ParentFrame;

    FunctionFrame(FunctionFrame *parent);
};

/**
 * Holds the internal state of the compiler.
 */
class CompilerContext {
    void declareStdLibFunctions();

public:
    Runtime &m_RuntimeRef;
    ///// IR gen
    llvm::LLVMContext &m_LLVMContext;
    llvm::IRBuilder<> &m_IRBuilder;
    llvm::Module &m_Module;
    std::vector<LoopBase> m_LoopLabels;
    llvm::Function *m_CurrentFunction = nullptr;
    std::atomic<size_t> &m_IdCounter;
    std::unordered_map<std::string, llvm::AllocaInst *> m_VariableMap;
    llvm::Argument *m_ClosureEnv = nullptr;
    ///// semantic analysis
    FunctionFrame *m_CurrentFunctionFrame = nullptr;
    size_t m_NumRecurArgs = 0;
    std::unordered_set<std::string> m_LocalBindings;

    ///// methods
    CompilerContext(Runtime &runtimeRef,
                    llvm::LLVMContext &llvmContext,
                    llvm::IRBuilder<> &irBuilder,
                    llvm::Module &module,
                    std::atomic<size_t> &idCounter);

    llvm::PointerType *pointerType() const;

    llvm::PointerType *pointerArrayType() const;

    // Creates a new basic block and a branch instruction to that block from the current block
    // and sets the IR insertion point to that block.
    void jumpToTmpBasicBlock();

    size_t nextId();

    // Creates a basic block
    llvm::BasicBlock *createBasicBlock(const std::string &name);

    bool currentBlockTerminated() const;
};
