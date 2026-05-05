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

std::filesystem::path util::resolve_file_path(const std::string &path) {
    std::string expanded = path;
    // expand ~ to home directory
    if (!expanded.empty() && expanded[0] == '~') {
        const char *home = getenv("HOME");
#ifdef _WIN32
        if (!home) {
            home = getenv("USERPROFILE");
        }
#endif
        if (home) {
            expanded = std::string(home) + expanded.substr(1);
        }
    }
    std::filesystem::path p(expanded);
    return std::filesystem::weakly_canonical(p);
}

std::unordered_map<char, std::string> symbol_escape_map = {
        {'+', "_PLUS_"},
        {'-', "_"},
        {'*', "_STAR_"},
        {'/', "_SLASH_"},
        {'=', "_EQ_"},
        {'<', "_LT_"},
        {'>', "_GT_"},
        {'?', "_QMARK_"},
        {'!', "_BANG_"},
        {'.', "_DOT_"},
        {'&', "_AMP_"},
        {'|', "_PIPE_"},
        {'^', "_CARET_"},
        {'%', "_PERCENT_"},
        {':', "_COLON_"},
        {'$', "_DOLLAR_"},
        {'@', "_AT_"},
        {'~', "_TILDE_"},
        {'#', "_HASH_"}
};

std::string util::munge_symbol(const std::string &name) {
    std::string normalized;
    for (char c: name) {
        if (symbol_escape_map.contains(c)) {
            normalized += symbol_escape_map[c];
        } else if (isalnum(c) || c == '_') {
            normalized += c;
        } else {
            normalized += "_";
        }
    }
    return normalized;
}
