#pragma once

#include <atomic>
#include <unordered_set>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "GCFrame.h"
#include "MemoryManager.h"
#include "types/Object.h"
#include "types/TCVar.h"

class Runtime {
    bool m_Initialized = false;
    bool m_InitInProgress = false;

    void init();

    void ensureInitialized();

    std::unique_ptr<llvm::orc::LLJIT> m_JIT;
    std::atomic<size_t> m_IdCounter = 0;
    std::unordered_map<std::string, Object *> m_GlobalVarStorage;
    std::unordered_set<const Object *> m_ConstantObjects;
    MemoryManager m_Heap;

    static std::unique_ptr<llvm::orc::LLJIT> createJIT();

    void defn(const std::string &name, CallFn fn);

    Runtime();

public:
    void registerConstant(const Object *obj);

    const std::unordered_set<const Object *> &getConstantObjects() const;

    GCRootFrame *m_RootStack = nullptr;

    Object *createObject(ObjectType type, void *data, CallFn callFn = nullptr, bool isStatic = false);

    Runtime(const Runtime &) = delete;

    Runtime &operator=(const Runtime &) = delete;

    static Runtime &getInstance();

    size_t nextId();

    const std::unordered_map<std::string, Object *> &getGlobalVarStorage() const;

    std::unordered_map<std::string, Object *> &getGlobalVarStorage();

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
