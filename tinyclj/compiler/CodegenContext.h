#pragma once

#include <atomic>
#include <unordered_set>
#include <vector>

#include "RecursionPoint.h"

struct CodegenContext {
    std::string m_ModuleName;
    std::unique_ptr<llvm::LLVMContext> m_LLVMContext;
    llvm::IRBuilder<> m_IRBuilder;
    std::unique_ptr<llvm::Module> m_Module;
    std::vector<RecursionPoint> m_LoopLabels;
    std::vector<llvm::Function *> m_CurrentFunctionStack;
    std::unordered_map<std::string, llvm::Value *> m_VariableMap;
    std::unordered_map<std::string, llvm::GlobalVariable *> m_GlobalVariableMap;
    std::vector<std::string> m_ModuleImports;
    llvm::Argument *m_ClosureEnv = nullptr;
    bool m_Optimized = false;

    llvm::PointerType *pointerType() const;

    llvm::PointerType *pointerArrayType() const;

    // Creates a new basic block and a branch instruction to that block from the current block
    // and sets the IR insertion point to that block.
    void jumpToTmpBasicBlock();

    // Creates a basic block
    llvm::BasicBlock *createBasicBlock(const std::string &name);

    bool currentBlockTerminated() const;

    CodegenContext(const std::string &moduleName);

    void linkModule();

    // LLVM global string cache for the current module, mapping string literals to their corresponding global variable
    // todo: do we need the size_t ID here?
    std::unordered_map<std::string, std::pair<size_t, llvm::GlobalVariable *>> m_GlobalStringCache;

    llvm::Value *registerGlobalString(const std::string &str);

    llvm::Function *currentFunction() const;

    llvm::Function *createModuleLoadFunction(const std::string &moduleName);

    llvm::GlobalVariable *getOrCreateGlobalVariable(const std::string &name);

    llvm::IntegerType *getArgcType() const;

private:
    void optimizeModule();

    void writeDebugModuleToFile();

    void writeBitcodeToFile();
};
