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
    llvm::LLVMContext &m_LLVMContext;
    llvm::IRBuilder<> &m_IRBuilder;
    llvm::Module &m_Module;
    std::vector<LoopBase> m_LoopLabels;
    FunctionFrame *m_CurrentFunctionFrame = nullptr;
    llvm::Function *m_CurrentFunction = nullptr;
    std::atomic<size_t> &m_IdCounter;
    std::unordered_map<std::string, llvm::AllocaInst *> m_VariableMap;
    llvm::Value *m_ClosureEnv = nullptr;
    std::unordered_set<std::string> m_LocalBindings;

    CompilerContext(Runtime &runtimeRef,
                    llvm::LLVMContext &llvmContext,
                    llvm::IRBuilder<> &irBuilder,
                    llvm::Module &module,
                    std::atomic<size_t> &idCounter);

    llvm::PointerType *objectPointerType() const;

    llvm::PointerType *objectPointerArrayType() const;

    // Creates a new basic block and a branch instruction to that block from the current block
    // and sets the IR insertion point to that block.
    void newTmpBasicBlock();

    size_t nextId();

    // Creates a basic block
    llvm::BasicBlock *createBasicBlock(const std::string &name);
};
