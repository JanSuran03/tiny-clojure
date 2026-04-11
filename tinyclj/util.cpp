#include "util.h"

std::string util::module_init_fn_name(const std::string &module_name) {
    return module_name + "__init";
}

std::string util::module_load_fn_name(const std::string &module_name) {
    return module_name + "__load";
}

std::string util::module_dependency_file_name(const std::string &module_name) {
    return module_name + ".deps";
}
