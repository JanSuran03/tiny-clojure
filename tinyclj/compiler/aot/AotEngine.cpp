#include <filesystem>
#include <fstream>
#include <iostream>

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"

#include "AotEngine.h"
#include "Module.h"
#include "runtime/Runtime.h"
#include "util.h"
#include "compiler/AnalyzerContext.h"
#include "compiler/CodegenContext.h"
#include "compiler/SemanticAnalyzer.h"
#include "compiler/aot/ModuleLoader.h"
#include "compiler/aot/ModuleUtil.h"
#include "compiler/ast/local-binding/CapturedLocalExpr.h"
#include "reader/LispReader.h"

std::string AotEngine::fullSourcePath(const std::string &moduleName) const {
    return m_SourceRoot + "/" + moduleName + ".clj";
}

std::string AotEngine::fullCompiledPath(const std::string &moduleName) const {
    return m_CompiledRoot + "/" + moduleName + ".bc";
}

std::string AotEngine::fullCompiledDebugPath(const std::string &moduleName) const {
    return m_CompiledRoot + "/" + moduleName + ".ll";
}

std::string AotEngine::fullDepsFileName(const std::string &moduleName) const {
    return m_CompiledRoot + "/" + util::module_dependency_file_name(moduleName);
}

bool shouldRecompile(const std::string &source_filename,
                     const std::string &output_filename,
                     bool forceRecompile) {
    using namespace std;
    if (!forceRecompile && filesystem::exists(output_filename)) {
        auto stdlib_source_modified_time = Runtime::getInstance().getSourceLastWriteTime();
        auto compiled_time = filesystem::last_write_time(output_filename);
        if (compiled_time < stdlib_source_modified_time) {
            if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
                std::cerr << "The TinyCLJ C++ source tree has been modified since the last compilation, recompiling: "
                          << output_filename << std::endl;
            }
            return true;
        }
        auto module_source_modified_time = filesystem::last_write_time(source_filename);
        // compiled file is newer than source file, no need to recompile
        if (compiled_time >= module_source_modified_time) {
            if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
                std::cerr << "Compiled file is up to date, skipping compilation: " << output_filename << std::endl;
            }
            return false;
        }
    }
    return true;
}

std::string AotEngine::compileModule(const std::string &moduleName, bool forceRecompile) {
    using namespace std;

    std::string source_filename = fullSourcePath(moduleName);
    string output_filename = fullCompiledPath(moduleName);
    string debug_ll_filename = fullCompiledDebugPath(moduleName);
    if (!shouldRecompile(source_filename, output_filename, forceRecompile)) {
        loadCompiledModule(moduleName);
        return output_filename;
    }

    ifstream ifs(source_filename);
    if (!ifs.is_open()) {
        throw runtime_error("Failed to open file: " + source_filename);
    }
    BufferedReader reader(ifs);
    Runtime &rt = Runtime::getInstance();
    bool old_compiling_aot = rt.m_CompilingAOT;
    rt.m_CompilingAOT = true;
    // we need a single context for the entire file since the file is loaded by invoking the generated load function,
    // thus, all top-level expressions share the same context
    AnalyzerContext analyzer_ctx;
    analyzer_ctx.m_ReferencedGlobalNamesStack.emplace_back();
    analyzer_ctx.m_ModuleImportsStack.emplace_back();
    std::vector<AExpr> top_level_exprs;
    while (true) {
        const Object *form = LispReader::read(reader);
        if (form == LispReader::eof_object()) {
            break;
        }

        AExpr expr = SemanticAnalyzer::analyze(analyzer_ctx, form);
        expr->eval();
        top_level_exprs.emplace_back(std::move(expr));
    }

    auto globals_names = std::move(analyzer_ctx.m_ReferencedGlobalNamesStack.back());
    analyzer_ctx.m_ReferencedGlobalNamesStack.pop_back();
    auto module_imports = std::move(analyzer_ctx.m_ModuleImportsStack.back());
    analyzer_ctx.m_ModuleImportsStack.pop_back();

    // create a new codegen context for the module and emit IR for all top-level expressions
    CodegenContext codegen_ctx(moduleName);
    codegen_ctx.m_Module->setSourceFileName(source_filename);

    Module module(moduleName, module_imports);

    //module.declareReferencedGlobals(codegen_ctx);
    auto llvm_globals = ModuleUtil::declareReferencedGlobals(codegen_ctx, globals_names);
    auto init_fn = ModuleUtil::initReferencedGlobals(codegen_ctx, moduleName, llvm_globals);
    //module.initReferencedGlobals(codegen_ctx, llvm_globals);

    if (Runtime::getInstance().m_CompilingAOT) {
        module.emitImportsFile();
    }

    llvm::Function *load_fn = codegen_ctx.createModuleLoadFunction(moduleName);

    codegen_ctx.m_GlobalVariableMapStack.emplace_back(std::move(llvm_globals));
    codegen_ctx.m_CurrentFunctionStack.emplace_back(load_fn);

    llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*codegen_ctx.m_LLVMContext, "entry", load_fn);
    codegen_ctx.m_IRBuilder.SetInsertPoint(entry_block);

    // emit IR for all top-level expressions
    for (const AExpr &expr: top_level_exprs) {
        expr->emitIR(codegen_ctx);
    }

    codegen_ctx.m_IRBuilder.CreateRetVoid();

    rt.m_CompilingAOT = old_compiling_aot;

    if (verifyModule(*codegen_ctx.m_Module, &llvm::errs()) != 0) {
        throw runtime_error("Generated LLVM IR is invalid");
    }

    ofstream ofs(output_filename);
    if (!ofs.is_open()) {
        throw runtime_error("Failed to open output file: " + output_filename);
    }
    error_code ec;
    llvm::raw_fd_ostream dest(output_filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        throw runtime_error("Could not open file: " + ec.message());
    } else {
        //codegen_ctx.m_Module->print(dest, nullptr);
        llvm::WriteBitcodeToFile(*codegen_ctx.m_Module, dest);
        dest.flush();

        if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
            llvm::errs() << "Compiled module written to " << output_filename << '\n';
        }
        // also write the human-readable LLVM IR for debugging
        ofstream debug_ofs(debug_ll_filename);
        if (!debug_ofs.is_open()) {
            throw runtime_error("Failed to open debug output file: " + debug_ll_filename);
        }
        llvm::raw_fd_ostream debug_dest(debug_ll_filename, ec, llvm::sys::fs::OF_None);
        if (ec) {
            throw runtime_error("Could not open debug file: " + ec.message());
        } else {
            codegen_ctx.m_Module->print(debug_dest, nullptr);
            debug_dest.flush();
            if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
                llvm::errs() << "Debug LLVM IR written to " << debug_ll_filename << '\n';
            }
        }
    }
    return output_filename;
}

void AotEngine::startLoading(const std::string &moduleName) {
    m_LoadingSet.insert(moduleName);
    m_LoadingStack.emplace_back(moduleName);
}

void AotEngine::finishLoading(const std::string &moduleName) {
    m_LoadingStack.pop_back();
    m_LoadingSet.erase(moduleName);
    m_LoadedFiles.insert(moduleName);
}

void AotEngine::loadCompiledModule(const std::string &moduleName, bool forceReload) {
    ModuleLoader::loadCompiledModule(moduleName, forceReload, true);
}

// todo: is this function even needed?
void AotEngine::loadCompiledFunctionModule(const std::string &moduleName, bool forceReload) {
    ModuleLoader::loadCompiledModule(moduleName, forceReload, false);
}
