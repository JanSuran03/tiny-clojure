/*
#include <fstream>
#include <iostream>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Support/Host.h"

#include "Module.h"
#include "runtime/Runtime.h"

*/
/*llvm::Function *Module::emitLoadModuleDependencies(CodegenContext &ctx,
                                                   const std::string &module_name,
                                                   const Globals &globals) {
    using namespace llvm;

    // void tc_runtime_load_module(const char *filename);
    FunctionType *load_file_fn_type = FunctionType::get(Type::getVoidTy(*ctx.m_LLVMContext),
                                                        {Type::getInt8PtrTy(*ctx.m_LLVMContext)},
                                                        false);
    FunctionCallee load_file_fn = ctx.m_Module->getOrInsertFunction("tc_runtime_load_module",
                                                                    load_file_fn_type);

    FunctionType *load_deps_fn_type = FunctionType::get(Type::getVoidTy(*ctx.m_LLVMContext),
                                                        {},
                                                        false);
    Function *load_deps_fn = Function::Create(load_deps_fn_type,
                                              Function::ExternalLinkage,
                                              util::module_load_deps_fn_name(module_name),
                                              *ctx.m_Module);

    BasicBlock *entry_block = BasicBlock::Create(*ctx.m_LLVMContext, "entry", load_deps_fn);
    ctx.m_IRBuilder.SetInsertPoint(entry_block);
    for (const std::string &global_name: globals) {// = util::file_name_from_var_name(global_name);
        Value *dependency_module_name_global_str = ctx.m_IRBuilder.CreateGlobalStringPtr(
                global_name, "dependency_module_name_" + global_name);
        ctx.m_IRBuilder.CreateCall(load_file_fn, {dependency_module_name_global_str});
    }
    ctx.m_IRBuilder.CreateRetVoid();
    return load_deps_fn;
}*//*


void Module::emitImportsFile(const std::string &module_name, const Module::Globals &globals) {
    std::string deps_file_name = Runtime::getInstance().getAotEngine().fullDepsFileName(module_name);
    std::ofstream ofs(deps_file_name);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open dependencies file for writing: " + deps_file_name);
    }
    for (const std::string &global_name: globals) {
        ofs << global_name << "\n";
    }
}

const std::unordered_map<std::string, llvm::GlobalVariable *>
Module::declareReferencedGlobals(CodegenContext &ctx, const Globals &globals) {
    using namespace llvm;
    std::unordered_map<std::string, llvm::GlobalVariable *> global_vars;
    for (const std::string &global_name: globals) {
        llvm::GlobalVariable *global_var = new llvm::GlobalVariable(
                *ctx.m_Module,
                ctx.pointerType(),
                false,
                llvm::GlobalValue::PrivateLinkage,
                ConstantPointerNull::get(ctx.pointerType()),
                "global_var_" + global_name);
        global_vars.emplace(global_name, global_var);
    }
    return global_vars;
}

llvm::Function *Module::initReferencedGlobals(CodegenContext &ctx,
                                              const std::string &module_name,
                                              const std::unordered_map<std::string, llvm::GlobalVariable *> &global_vars) {
    using namespace llvm;

    Type *objPtrTy = ctx.pointerType();
    Type *charPtrTy = Type::getInt8PtrTy(*ctx.m_LLVMContext);
    FunctionType *declare_var_fn_type = FunctionType::get(objPtrTy, {charPtrTy}, false);
    Function *declare_var_fn = Function::Create(declare_var_fn_type,
                                                Function::ExternalLinkage,
                                                "tc_runtime_declare_var",
                                                *ctx.m_Module);
    FunctionType *init_fn_type = FunctionType::get(Type::getVoidTy(*ctx.m_LLVMContext), {}, false);
    Function *init_fn = Function::Create(init_fn_type,
                                         Function::ExternalLinkage,
                                         util::module_init_fn_name(module_name),
                                         *ctx.m_Module);
    BasicBlock *entry_block = BasicBlock::Create(*ctx.m_LLVMContext, "entry", init_fn);
    ctx.m_IRBuilder.SetInsertPoint(entry_block);
    for (const auto &[name, global_var]: global_vars) {
        Value *name_global_str = ctx.m_IRBuilder.CreateGlobalStringPtr(name, "global_name_" + name);
        Value *var_obj = ctx.m_IRBuilder.CreateCall(declare_var_fn, {name_global_str}, "var_obj");
        ctx.m_IRBuilder.CreateStore(var_obj, global_var);
    }
    ctx.m_IRBuilder.CreateRetVoid();
    return init_fn;
}

void Module::loadCompiledModule(const std::string &module_name) {
    std::string full_compiled_path = Runtime::getInstance().getAotEngine().fullCompiledPath(module_name);
    auto ts_ctx = std::make_unique<llvm::LLVMContext>();
    ts_ctx->enableOpaquePointers();

    auto buffer_or_err = llvm::MemoryBuffer::getFile(full_compiled_path);
    if (!buffer_or_err) {
        throw std::runtime_error("Failed to read bitcode file: "
                                 + module_name
                                 + "\nReason: "
                                 + buffer_or_err.getError().message());
    }
    auto module_or_err = parseBitcodeFile(buffer_or_err->get()->getMemBufferRef(), *ts_ctx);
    if (!module_or_err) {
        throw std::runtime_error("Failed to parse bitcode file: "
                                 + module_name
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

    using LoadFnType = void (*)();

    std::string init_globals_function_name = util::module_init_fn_name(module_name);
    auto init_globals_fn = reinterpret_cast<LoadFnType>(jit->lookup(init_globals_function_name)->getAddress());
    if (!init_globals_fn) {
        throw std::runtime_error("Failed to find init globals function: " + init_globals_function_name);
    }
    init_globals_fn();

    std::string load_function_name = util::module_load_fn_name(module_name);
    auto load_fn = reinterpret_cast<LoadFnType>(jit->lookup(load_function_name)->getAddress());
    if (!load_fn) {
        throw std::runtime_error("Failed to find load function: " + load_function_name);
    }
    load_fn();
}
*/
