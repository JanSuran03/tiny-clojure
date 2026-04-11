#include <fstream>

#include "FunctionModule.h"
#include "util.h"
#include "runtime/Runtime.h"

FunctionModule::FunctionModule(std::string name,
                               std::unordered_set<std::string> imports)
        : m_Name(std::move(name)),
          m_Imports(std::move(imports)) {}

void FunctionModule::emitImportsFile() {
    std::string deps_file_name = Runtime::getInstance().getAotEngine().fullDepsFileName(m_Name);
    std::ofstream ofs(deps_file_name);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open dependencies file for writing: " + deps_file_name);
    }
    for (const std::string &global_name: m_Imports) {
        ofs << global_name << "\n";
    }
}

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
