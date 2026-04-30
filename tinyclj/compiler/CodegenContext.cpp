#include <fstream>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include "llvm/Bitcode/BitcodeWriter.h"
#include <llvm/IR/PassManager.h>
#include "llvm/IR/Verifier.h"
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>

#include "util.h"
#include "CodegenContext.h"
#include "runtime/Runtime.h"

CodegenContext::CodegenContext(const std::string &moduleName)
        : m_ModuleName(moduleName),
          m_LLVMContext(std::make_unique<llvm::LLVMContext>()),
          m_IRBuilder(*m_LLVMContext),
          m_Module(std::make_unique<llvm::Module>(moduleName, *m_LLVMContext)) {
    m_LLVMContext->enableOpaquePointers();
}

llvm::PointerType *CodegenContext::pointerType() const {
    return llvm::PointerType::get(*m_LLVMContext, 0);
}

llvm::PointerType *CodegenContext::pointerArrayType() const {
    return llvm::PointerType::get(pointerType(), 0);
}

void CodegenContext::jumpToTmpBasicBlock() {
    auto block_id = "block__" + std::to_string(Runtime::nextId());
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_LLVMContext, block_id, currentFunction());
    m_IRBuilder.CreateBr(block);
    m_IRBuilder.SetInsertPoint(block);
}

llvm::BasicBlock *CodegenContext::createBasicBlock(const std::string &name) {
    return llvm::BasicBlock::Create(*m_LLVMContext,
                                    name,
                                    currentFunction());
}

bool CodegenContext::currentBlockTerminated() const {
    return m_IRBuilder.GetInsertBlock()->getTerminator() != nullptr;
}

void CodegenContext::optimizeModule() {
    llvm::LoopAnalysisManager loop_analysis_manager;
    llvm::FunctionAnalysisManager function_analysis_manager;
    llvm::CGSCCAnalysisManager cgscc_analysis_manager;
    llvm::ModuleAnalysisManager module_analysis_manager;

    llvm::PassBuilder pb;

    pb.registerModuleAnalyses(module_analysis_manager);
    pb.registerCGSCCAnalyses(cgscc_analysis_manager);
    pb.registerFunctionAnalyses(function_analysis_manager);
    pb.registerLoopAnalyses(loop_analysis_manager);
    pb.crossRegisterProxies(loop_analysis_manager,
                            function_analysis_manager,
                            cgscc_analysis_manager,
                            module_analysis_manager);

    llvm::ModulePassManager module_pass_manager = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    module_pass_manager.run(*m_Module, module_analysis_manager);
    m_Optimized = true;
}

void CodegenContext::writeDebugModuleToFile() {
    std::error_code ec;
    llvm::raw_fd_ostream dest(Runtime::getInstance().getAotEngine().fullCompiledDebugPath(m_ModuleName, m_Optimized),
                              ec,
                              llvm::sys::fs::OF_None);
    if (ec) {
        throw std::runtime_error("Could not open debug file: " + ec.message());
    } else {
        m_Module->print(dest, nullptr);
        dest.flush();
        if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
            llvm::errs() << "Debug LLVM IR written to "
                         << Runtime::getInstance().getAotEngine().fullCompiledDebugPath(m_ModuleName) << '\n';
        }
    }
}

void CodegenContext::writeBitcodeToFile() {
    std::string output_filename = Runtime::getInstance().getAotEngine().fullCompiledPath(m_ModuleName, m_Optimized);
    std::error_code ec;
    llvm::raw_fd_ostream dest(output_filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        throw std::runtime_error("Could not open file: " + ec.message());
    } else {
        llvm::WriteBitcodeToFile(*m_Module, dest);
        dest.flush();
        if constexpr (Runtime::st_DebugFlags & Runtime::DEBUG_LOADER) {
            llvm::errs() << "Compiled module written to " << output_filename << '\n';
        }
    }
}

void CodegenContext::linkModule() {
    if (llvm::verifyModule(*m_Module, &llvm::errs())) {
        m_Module->dump();
        throw std::runtime_error("Module verification failed");
    }

    if (Runtime::getInstance().m_CompilingAOT) {
        writeBitcodeToFile();
        writeDebugModuleToFile();
    }
    optimizeModule();

    if (llvm::verifyModule(*m_Module, &llvm::errs())) {
        m_Module->dump();
        throw std::runtime_error("Optimized module verification failed");
    }

    if (Runtime::getInstance().m_CompilingAOT) {
        writeBitcodeToFile();
        writeDebugModuleToFile();
    }

    llvm::orc::ThreadSafeModule tsm(std::move(m_Module), std::move(m_LLVMContext));

    if (auto err = Runtime::getInstance().getJIT()->addIRModule(std::move(tsm))) {
        throw std::runtime_error("Failed to add module to JIT: " + llvm::toString(std::move(err)));
    }
}

llvm::Value *CodegenContext::registerGlobalString(const std::string &str) {
    using namespace llvm;
    GlobalVariable *globalStr;
    size_t strId;
    if (auto it = m_GlobalStringCache.find(str); it != m_GlobalStringCache.end()) {
        globalStr = it->second.second;
        strId = it->second.first;
    } else {
        strId = Runtime::getInstance().nextId();
        globalStr = new llvm::GlobalVariable(
                *m_Module,
                ArrayType::get(Type::getInt8Ty(*m_LLVMContext), str.size() + 1),
                true, // isConstant
                GlobalValue::PrivateLinkage,
                ConstantDataArray::getString(*m_LLVMContext, str),
                "str_" + std::to_string(strId)
        );
        m_GlobalStringCache.emplace(str, std::make_pair(strId, globalStr));
    }
    return m_IRBuilder.CreateBitCast(globalStr, Type::getInt8PtrTy(*m_LLVMContext), "strptr_" + std::to_string(strId));
}

llvm::Function *CodegenContext::currentFunction() const {
    return m_CurrentFunctionStack.back();
}

llvm::Function *CodegenContext::createModuleLoadFunction(const std::string &moduleName) {
    llvm::FunctionType *load_fn_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_LLVMContext), {}, false);
    llvm::Function *load_fn = llvm::Function::Create(load_fn_type,
                                                     llvm::Function::ExternalLinkage,
                                                     util::module_load_fn_name(moduleName),
                                                     *m_Module);
    return load_fn;
}

llvm::GlobalVariable *CodegenContext::getOrCreateGlobalVariable(const std::string &name) {
    using namespace llvm;
    if(auto it = m_GlobalVariableMap.find(name); it != m_GlobalVariableMap.end()) {
        return it->second;
    } else {
        GlobalVariable *global_var = new GlobalVariable(
                *m_Module,
                pointerType(),
                false,
                GlobalValue::PrivateLinkage,
                ConstantPointerNull::get(pointerType()),
                "var_" + name);
        m_GlobalVariableMap.emplace(name, global_var);
        return global_var;
    }
}
