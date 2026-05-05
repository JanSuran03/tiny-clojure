#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace util {
    /// A function that initializes a module by loading its dependencies and initializing its global variables.
    std::string module_init_fn_name(const std::string &module_name);

    /// A function that loads a module by initializing it and then executing its top-level code.
    std::string module_load_fn_name(const std::string &module_name);

    /// The name of the file that contains dependencies for the compiled module.
    std::string module_dependency_file_name(const std::string &module_name);

    /// Resolves a file path by expanding ~ to the home directory and converting it to an absolute path.
    std::filesystem::path resolve_file_path(const std::string &path);

    /// Normalizes a symbol name by replacing special characters with underscores
    std::string munge_symbol(const std::string &name);
}
