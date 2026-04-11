#pragma once

#include <string>

namespace util {
    /// A function that initializes a module by loading its dependencies and initializing its global variables.
    std::string module_init_fn_name(const std::string &module_name);

    /// A function that loads a module by initializing it and then executing its top-level code.
    std::string module_load_fn_name(const std::string &module_name);

    /// The name of the file that contains dependencies for the compiled module.
    std::string module_dependency_file_name(const std::string &module_name);
}
