#pragma once

#include <atomic>

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
    std::atomic<int64_t> m_LabelCounter = 0;
    std::unordered_map<std::string, llvm::Value *> m_VariableMap;

    CompilerContext(llvm::LLVMContext &llvmContext,
                    llvm::IRBuilder<> &irBuilder,
                    llvm::Module &module);
};
