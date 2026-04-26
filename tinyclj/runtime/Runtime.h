#pragma once

#include <atomic>
#include <filesystem>
#include <unordered_set>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "compiler/aot/AotEngine.h"
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
    MemoryManager m_Heap;

    static std::unique_ptr<llvm::orc::LLJIT> createJIT();

    template<CallFn Fn>
    struct BuiltinFunctionVTable {
        static constexpr MethodTable value = {
                .m_CallFn = Fn,
                .m_ToStringFn = nullptr,
                .m_ToEdnFn = nullptr
        };
    };

    template<CallFn Fn>
    void defn(const std::string &name);

    Runtime();

    AotEngine m_AotEngine;

    // last write time of the C++ source
    std::filesystem::file_time_type m_SourceLastWriteTime;

    static std::filesystem::file_time_type computeLastSourceWriteTime();

public:
    bool m_CompilingAOT = false;
    bool m_SuppressReplWelcome = false;

    std::filesystem::file_time_type getSourceLastWriteTime() const;

    AotEngine &getAotEngine();

    static constexpr uint64_t DEBUG_LOADER = 1ULL << 0;
    static constexpr uint64_t st_DebugFlags = 0 /*& DEBUG_LOADER*/;

    GCRootFrame *m_RootStack = nullptr;

    Object *createObject(ObjectType type, void *data, const MethodTable *methodTable, bool isStatic = false);

    Runtime(const Runtime &) = delete;

    Runtime &operator=(const Runtime &) = delete;

    static Runtime &getInstance();

    static size_t nextId();

    const std::unordered_map<std::string, Object *> &getGlobalVarStorage() const;

    std::unordered_map<std::string, Object *> &getGlobalVarStorage();

    Object *declareVar(const std::string &name);

    Object *getVar(const std::string &name) const;

    std::unique_ptr<llvm::orc::LLJIT> &getJIT();

    static void repl();

    static const Object *eval(const Object *form);

    static const Object *read();

    static const Object *readString(const std::string &input);

    static const Object *loadString(const std::string &input);

    static const Object *slurp(const std::string &filename);

    static void spit(const std::string &filename, const std::string &content);

    static const Object *loadStream(std::istream &stream);

    static const Object *loadFile(const std::string &filename);
};

extern "C" {
Object *tc_runtime_declare_var(const char *name);

void tc_runtime_load_module(const char *moduleName);
}
