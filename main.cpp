#include <filesystem>
#include <iostream>

#include "Runtime.h"
#include "compiler/SemanticAnalyzer.h"
#include "runtime/rt.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
#include "types/TCInteger.h"

#include "llvm/Support/TargetSelect.h"

const std::string out_dir = "out";

std::string compile_cpp(const std::string &full_source_path) {
    std::string source_dir = full_source_path.substr(0, full_source_path.find_last_of('/'));
    std::string source_filename = full_source_path.substr(full_source_path.find_last_of('/') + 1);
    std::string output_dir = std::string(out_dir) + "/" + source_dir;
    std::string output_path = output_dir + "/" + source_filename + ".o";
    system(std::string("mkdir -p ").append(output_dir).c_str());
    system(std::string("clang++ ").append(full_source_path).append(" -c -o ").append(output_path).c_str());
    return output_path;
}

int main() {
    // C++ sources are located in tinyclj/types/*.cpp, compile them to .o files to link them later
    std::string source_dir = "tinyclj/types";
    std::vector<std::string> compiled_files;
    for (const auto &file: std::filesystem::directory_iterator(source_dir)) {
        if (file.path().extension() == ".cpp") {
            std::string full_source_path = file.path().string();
            std::string output_path = compile_cpp(full_source_path);
            std::cout << "Compiled " << full_source_path << " to " << output_path << std::endl;
            compiled_files.push_back(output_path);
        }
    }

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // (let* (a 1) a)
    //const Object *bindings = tc_list_cons(tc_symbol_new("a"),
    //                                      tc_list_cons(tc_integer_new(1), empty_list()));
    //const Object *form = tc_list_cons(tc_symbol_new("let*"),
    //                                  tc_list_cons(bindings,
    //                                               tc_list_cons(tc_symbol_new("a"),
    //                                                            empty_list())));

    Runtime runtime(compiled_files);
    runtime.repl();

    //Object *res = runtime.eval(form);
    //tinyclj_rt_print(res);

    //std::cout << "Hello, World!" << std::endl;

    return 0;
}
