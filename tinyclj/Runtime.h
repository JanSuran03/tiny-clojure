#pragma once

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "compiler/CompilerContext.h"
#include "types/Object.h"

class Runtime {
    std::unique_ptr<llvm::orc::LLJIT> m_JIT;

    static std::unique_ptr<llvm::orc::LLJIT> createJIT();
public:
    Runtime(const std::vector<std::string> &objectFiles);

    std::unique_ptr<llvm::orc::LLJIT> &getJIT();

    Object *eval(const Object *form);
};
