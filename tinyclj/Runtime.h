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
    std::unordered_map<std::string, Object *> m_GlobalVarStorage;

    static std::unique_ptr<llvm::orc::LLJIT> createJIT();

    void init();

public:
    Runtime(const std::vector<std::string> &objectFiles);

    Object *declareVar(const std::string &name);

    Object *getVar(const std::string &name) const;

    std::unique_ptr<llvm::orc::LLJIT> &getJIT();

    Object *eval(const Object *form);

    void repl();

    static const Object *readString(const std::string &input);

    const Object *loadString(const std::string &input);

    const Object *loadStream(std::istream &stream);

    const Object *loadFile(const std::string &filename);
};
