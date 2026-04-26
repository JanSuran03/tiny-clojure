#include <fstream>

#include "FunctionModule.h"
#include "util.h"
#include "runtime/Runtime.h"

FunctionModule::FunctionModule(std::string name,
                               std::unordered_set<std::string> imports)
        : Module(std::move(name), std::move(imports)) {}

void FunctionModule::writeModule(CodegenContext &ctx) {
    std::string full_compiled_path = Runtime::getInstance().getAotEngine().fullCompiledPath(m_Name);
    std::error_code ec;
    llvm::raw_fd_ostream dest(full_compiled_path, ec, llvm::sys::fs::OF_None);
    if (ec) {
        throw std::runtime_error("Could not open file: " + ec.message());
    } else {
        ctx.m_Module->print(dest, nullptr);
        dest.flush();
    }
}

void FunctionModule::createModuleVtable(CodegenContext &ctx, llvm::Function *callFn) {
    // create a global variable for the vtable and initialize it with the call function pointer
    llvm::StructType *vtableStructType = Object::getMethodTableStructType(*ctx.m_LLVMContext);
    llvm::Constant *callFnPtr = llvm::ConstantExpr::getBitCast(callFn, llvm::PointerType::get(*ctx.m_LLVMContext, 0));
    llvm::Constant *toStringFnPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(*ctx.m_LLVMContext, 0));
    llvm::Constant *toEdnFnPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(*ctx.m_LLVMContext, 0));
    llvm::Constant *vtableInit = llvm::ConstantStruct::get(vtableStructType, {callFnPtr, toStringFnPtr, toEdnFnPtr});
    llvm::GlobalVariable *vtableGlobalVar = new llvm::GlobalVariable(
            *ctx.m_Module,
            vtableStructType,
            true, // isConstant
            llvm::GlobalValue::ExternalLinkage,
            vtableInit,
            m_VtableName);
}
