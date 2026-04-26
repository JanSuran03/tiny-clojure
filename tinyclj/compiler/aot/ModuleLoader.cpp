#include <fstream>
#include <iostream>
#include <memory>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MemoryBuffer.h"

#include "ModuleLoader.h"
#include "util.h"
#include "runtime/Runtime.h"

void ModuleLoader::loadCompiledModule(const std::string &moduleName, bool forceReload, bool isMainModule) {
    AotEngine &aot_engine = Runtime::getInstance().getAotEngine();
    if (!aot_engine.m_LoadingStack.empty() && moduleName == aot_engine.m_LoadingStack.back()) {
        // allow reentrant load of the same module (todo: is this correct?)
        return;
    }
    if (aot_engine.m_LoadingSet.contains(moduleName)) {
        size_t cycle_start_index = 0;
        for (size_t i = 0; i < aot_engine.m_LoadingStack.size(); i++) {
            if (aot_engine.m_LoadingStack[i] == moduleName) {
                cycle_start_index = i;
                break;
            }
        }
        std::string cycle = moduleName;
        for (size_t i = cycle_start_index + 1; i < aot_engine.m_LoadingStack.size(); i++) {
            cycle += " -> " + aot_engine.m_LoadingStack[i];
        }
        throw std::runtime_error("cyclic dependency detected while loading module: " + cycle);
    }
    if (!forceReload && aot_engine.m_LoadedFiles.contains(moduleName)) {
        if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
            std::cerr << "FileModule has already been loaded, skipping: " << moduleName << std::endl;
        }
        return;
    }

    if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
        std::cerr << "Loading module: " << moduleName << std::endl;
    }

    aot_engine.startLoading(moduleName);

    try {
        std::string full_deps_path = aot_engine.fullDepsFileName(moduleName);
        std::ifstream deps_ifs(full_deps_path);
        if (!deps_ifs.is_open()) {
            throw std::runtime_error("Failed to open dependencies module: " + full_deps_path);
        }
        std::string dep_module_name;
        while (std::getline(deps_ifs, dep_module_name)) {
            if (!dep_module_name.empty()) {
                loadCompiledModule(dep_module_name, forceReload, false);
            }
        }

        std::string full_compiled_path = aot_engine.fullCompiledPath(moduleName);
        auto ts_ctx = std::make_unique<llvm::LLVMContext>();
        ts_ctx->enableOpaquePointers();

        auto buffer_or_err = llvm::MemoryBuffer::getFile(full_compiled_path);
        if (!buffer_or_err) {
            throw std::runtime_error("Failed to read bitcode file: "
                                     + moduleName
                                     + "\nReason: "
                                     + buffer_or_err.getError().message());
        }
        auto module_or_err = llvm::parseBitcodeFile(buffer_or_err->get()->getMemBufferRef(), *ts_ctx);
        if (!module_or_err) {
            throw std::runtime_error("Failed to parse bitcode file: "
                                     + moduleName
                                     + "\nReason: "
                                     + llvm::toString(module_or_err.takeError()));
        }
        auto module = std::move(*module_or_err);

        auto &jit = Runtime::getInstance().getJIT();
        module->setDataLayout(jit->getDataLayout());
        module->setTargetTriple(llvm::sys::getProcessTriple());

        llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(ts_ctx));
        if (auto err = jit->addIRModule(std::move(tsm))) {
            throw std::runtime_error("Failed to add module to JIT: " + llvm::toString(std::move(err)));
        }

        using InitFnType = void (*)();

        std::string init_globals_function_name = util::module_init_fn_name(moduleName);
        auto init_globals_fn = reinterpret_cast<InitFnType>(jit->lookup(init_globals_function_name)->getAddress());
        if (!init_globals_fn) {
            throw std::runtime_error("Failed to find init globals function: " + init_globals_function_name);
        }
        init_globals_fn();

        if (isMainModule) {
            std::string load_function_name = util::module_load_fn_name(moduleName);
            auto load_fn = reinterpret_cast<InitFnType>(jit->lookup(load_function_name)->getAddress());
            if (!load_fn) {
                throw std::runtime_error("Failed to find load function: " + load_function_name);
            }
            load_fn();
        }
    } catch (...) {
        aot_engine.finishLoading(moduleName);
        throw;
    }
    aot_engine.finishLoading(moduleName);
}
