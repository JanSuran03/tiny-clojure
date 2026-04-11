#include <fstream>

#include "Module.h"
#include "runtime/Runtime.h"

Module::Module(std::string name,
               std::unordered_set<std::string> imports)
        : m_Name(std::move(name)),
          m_Imports(std::move(imports)) {}

// todo: use the same function for both function and regular modules
void Module::emitImportsFile() {
    std::string deps_file_name = Runtime::getInstance().getAotEngine().fullDepsFileName(m_Name);
    std::ofstream ofs(deps_file_name);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open dependencies file for writing: " + deps_file_name);
    }
    for (const std::string &global_name: m_Imports) {
        ofs << global_name << "\n";
    }
}
