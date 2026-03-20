#include <filesystem>
#include <iostream>
#include <utility>

#include "Runtime.h"
#include "reader/LispReader.h"
#include "runtime/rt.h"

const std::string out_dir = "out";

#include "llvm/Support/TargetSelect.h"

std::string compile_cpp(const std::string &full_source_path) {
    std::string source_dir = full_source_path.substr(0, full_source_path.find_last_of('/'));
    std::string source_filename = full_source_path.substr(full_source_path.find_last_of('/') + 1);
    std::string output_dir = std::string(out_dir) + "/" + source_dir;
    std::string output_path = output_dir + "/" + source_filename + ".o";
    system(std::string("mkdir -p ").append(output_dir).c_str());
    system(std::string("clang++ ").append(full_source_path).append(" -c -o ").append(output_path).c_str());
    return output_path;
}

bool test_eval(Runtime &runtime,
               const std::string &input,
               const std::string &expected_console_output,
               const std::string &expected_result_str) {
    std::istringstream iss(input);
    std::ostringstream console_oss;
    std::ostringstream result_oss;
    BufferedReader rdr(iss);
    const Object *obj = LispReader::read(rdr);

    auto orig_rdbuf = std::cout.rdbuf();
    // redirect std::cout to capture console output
    std::cout.rdbuf(console_oss.rdbuf());
    const Object *res = runtime.eval(obj);
    // redirect std::cout back to capture the result printing output
    std::cout.rdbuf(result_oss.rdbuf());
    tinyclj_rt_print(res, 1, &res);
    // restore std::cout
    std::cout.rdbuf(orig_rdbuf);
    return console_oss.str() == expected_console_output && result_oss.str() == expected_result_str;
}

template<typename Fn, typename ErrFn, typename... Args>
void test(const std::string &name, Fn test_fn, ErrFn err_fn, const std::vector<std::tuple<Args...>> &cases) {
    unsigned passed = 0;

    for (size_t i = 0; i < cases.size(); i++) {
        try {
            if (std::apply(test_fn, cases[i])) {
                passed++;
                std::cout << "Passed test '" << name << "' (" << (i + 1) << '/' << cases.size() << ')' << std::endl;
            } else {
                std::cerr << "Failed test '" << name << "' (" << (i + 1) << '/' << cases.size() << "): "
                          << std::apply(err_fn, cases[i]) << '\n';
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed test '" << name << "' (" << (i + 1) << '/' << cases.size() <<
                      ") due to an uncaught exception: "
                      << std::apply(err_fn, cases[i]) << '\n' << e.what() << '\n';
        }
    }

    if (passed != cases.size()) {
        std::cerr << "Not all test cases in test '" << name << "' passed, exiting...\n";
        exit(1);
    } else {
        std::cout << "Passed all " << cases.size() << " test cases in test '" << name << "'.\n";
    }
}

int main() {
    std::string source_dir = std::string(PROJECT_SOURCE_DIR) + "/tinyclj/types";
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

    Runtime runtime(compiled_files);

    test("test eval",
         [&runtime](const std::string &input,
                    const std::string &expected_console_output,
                    const std::string &expected_result_str) {
             return test_eval(runtime, input, expected_console_output, expected_result_str);
         },
         [](const std::string &input,
            const std::string &expected_console_output,
            const std::string &expected_output) {
             return "eval '" + input
                    + "' -> '" + expected_output
                    + "', console output: '" + expected_console_output + "'";
         },
         std::vector<std::tuple<std::string, std::string, std::string>>
                 {{"67",                 "",                                "67"},
                  {"6.9",                "",                                "6.9"},
                  {"nil",                "",                                "nil"},
                         // todo: EDN vs REPL printing - this one should be printed as EDN
                  {"\"hello world!\"",   "",                                "hello world!"},
                  {"true",               "",                                "true"},
                  {"false",              "",                                "false"},
                  {"(do 1 2)",           "",                                "2"},
                  {"(do)",               "",                                "nil"},
                  {"(if true 1 2)",      "",                                "1"},
                  {"(if false 1 2)",     "",                                "2"},
                  {"(if nil 1 2)",       "",                                "2"},
                  {"(if 1 1 2)",         "",                                "1"},
                  {"(if 0 1 2)",         "",                                "1"},
                  {"(let (a 1"
                   "      b 2)"
                   "  a)",               "",                                "1"},
                  {"(+ 1 2)",            "",                                "3"},
                  {"(let (a 1"
                   "      b 2)"
                   "  (+ a b))",         "",                                "3"},
                  {"(let (add +)"
                   "  (add 1 2))",       "",                                "3"},
                  {"(let (add +"
                   "      a 1"
                   "      b 2)"
                   "  (add a b))",       "",                                "3"},
                  {"((fn (a)"
                   "   (+ a a))"
                   " 2)",                "",                                "4"
                  },
                  {"(let (add (fn (x y)"
                   "            (+ x y)))"
                   "  (add 1 2))",
                                         "",                                "3"},
                  {"(let (a 1"
                   "      adder (fn (b)"
                   "               (+ a b)))"
                   "  (adder 2))",       "",                                "3"},
                  {"(let (make-adder (fn (a)"
                   "                   (fn (b) (+ a b)))"
                   "       adder-2 (make-adder 2))"
                   "  (adder-2 3))",     "",                                "5"},
                  {"(let (make-adder (fn (a)"
                   "                   (fn (b) (+ a b)))"
                   "      make-multi-adder (fn (a b)"
                   "                         (let (adder-a (make-adder a)"
                   "                               adder-b (make-adder b))"
                   "                           (fn (c)"
                   "                             (adder-b (adder-a c)))))"
                   "       adder-2-3 (make-multi-adder 2 3))"
                   "  (adder-2-3 4))",   "",                                "9"},
                  {"(do (def countdown"
                   "      (fn (x)"
                   "        (if (zero? x)"
                   "          (print \"Done.\\n\")"
                   "          (do (print x)"
                   "              (print \"...\\n\")"
                   "              (countdown (- x 1))))))"
                   "    (countdown 3))", "3...\n2...\n1...\nDone.\n",       "nil"},
                  {"((fn (x y)"
                   "  (if (zero? x)"
                   "    (print \"Done.\\n\")"
                   "    (do (print x)"
                   "        (print \" \")"
                   "        (print y)"
                   "        (print \"...\\n\")"
                   "        (recur (- x 1)"
                   "               (+ y 1)))))"
                   "  3 3)",             "3 3...\n2 4...\n1 5...\nDone.\n", "nil"}}

    );
}
