#pragma once

#include <atomic>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "types/TCVar.h"
#include "compiler/CompilerContext.h"
#include "types/Object.h"

class Runtime {
    std::unique_ptr<llvm::orc::LLJIT> m_JIT;
    std::atomic<size_t> m_IdCounter = 0;
    std::unordered_map<std::string, TCVar *> m_GlobalVarStorage;

    static std::unique_ptr<llvm::orc::LLJIT> createJIT();

    void init();

public:
    Runtime(const std::vector<std::string> &objectFiles);

    TCVar *declareVar(const std::string &name);

    TCVar *getVar(const std::string &name) const;

    std::unique_ptr<llvm::orc::LLJIT> &getJIT();

    Object *eval(const Object *form);

    void repl();
};
