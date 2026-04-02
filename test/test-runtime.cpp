#include <iostream>
#include <utility>

#include "llvm/Support/TargetSelect.h"

#include "runtime/Runtime.h"
#include "reader/LispReader.h"
#include "runtime/rt.h"

struct TestCase {
    std::string input,
            expected_console_output,
            expected_result_str,
            description;

    TestCase(std::string input,
             std::string expected_console_output,
             std::string expected_result_str,
             std::string description) :
            input(std::move(input)),
            expected_console_output(std::move(expected_console_output)),
            expected_result_str(std::move(expected_result_str)),
            description(std::move(description)) {}
};

bool test_eval(Runtime &runtime,
               const TestCase &test_case) {
    std::istringstream iss(test_case.input);
    std::ostringstream console_oss;
    BufferedReader rdr(iss);
    const Object *obj = LispReader::read(rdr);

    auto orig_rdbuf = std::cout.rdbuf();
    // redirect std::cout to capture console output
    std::cout.rdbuf(console_oss.rdbuf());
    const Object *res = runtime.eval(obj);
    // restore std::cout
    std::cout.rdbuf(orig_rdbuf);
    Object *as_edn = tinyclj_rt_to_edn(nullptr, 1, &res);
    std::string result_string = static_cast<TCString *>(as_edn->m_Data)->m_Value;
    return console_oss.str() == test_case.expected_console_output
           && result_string == test_case.expected_result_str;
}

template<typename Fn, typename ErrFn>
void test(const std::string &name, Fn test_fn, ErrFn err_fn, const std::vector<TestCase> &cases) {
    unsigned passed = 0;

    for (size_t i = 0; i < cases.size(); i++) {
        try {
            if (test_fn(cases[i])) {
                passed++;
            } else {
                std::cerr << "Failed test '" << name << "' (" << (i + 1) << '/' << cases.size() << "): "
                          << err_fn(cases[i]) << '\n';
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed test '" << name << "' (" << (i + 1) << '/' << cases.size() <<
                      ") due to an uncaught exception: "
                      << err_fn(cases[i]) << '\n' << e.what() << '\n';
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
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    Runtime &runtime = Runtime::getInstance();

    std::vector<TestCase> cases = {
            TestCase("67", "", "67", "eval integer literal"),
            TestCase("6.9", "", "6.900000", "eval double literal"),
            TestCase("nil", "", "nil", "eval nil literal"),
            TestCase("\"hello world!\"", "", "\"hello world!\"", "eval string literal"),
            TestCase("true", "", "true", "eval true literal"),
            TestCase("false", "", "false", "eval false literal"),
            TestCase("(do)", "", "nil", "eval do with no expressions"),
            TestCase("(do 1 2)", "", "2", "eval do with multiple expressions"),
            TestCase("(if true 1 2)", "", "1", "eval if with true condition"),
            TestCase("(if false 1 2)", "", "2", "eval if with false condition"),
            TestCase("(if nil 1 2)", "", "2", "eval if with nil condition"),
            TestCase("(if 1 1 2)", "", "1", "eval if with non-nil condition"),
            TestCase("(if 0 1 2)", "", "1", "eval if with non-nil condition"),
            TestCase("(if true 1)", "", "1", "eval if with missing else branch"),
            TestCase("(if false 1)", "", "nil", "eval if with missing else branch and false condition"),
            TestCase("(let () 1)", "", "1", "eval let with empty bindings"),
            TestCase("(let (a 1) a)", "", "1", "eval let with a simple binding"),
            TestCase("(let (a 1 b 2) a)", "", "1", "eval let with multiple bindings, referencing the first binding"),
            TestCase("(let (a 1 a 2) a)", "", "2", "eval let with with a shadowed simple binding"),
            TestCase("(let (a 1 b a) b)", "", "1",
                     "eval let with with a shadowed binding referencing the previous binding"),
            TestCase("(let (add +) (add 1 2))", "", "3", "eval let with a bound global variable"),
            TestCase("(let (+ -) (+ 5 2))", "", "3", "eval let with a shadowed global variable"),
            TestCase("((fn (a) (+ a a)) 2)", "", "4", "eval anonymous function"),
            TestCase(R"##(
(let (add (fn (x y)
       (+ x y)))
  (add 1 2))
)##", "", "3", "let with a bound anonymous function"),
            TestCase(R"##(
(let (a 1
      adder (fn (b)
              (+ a b)))
  (adder 2))
)##", "", "3", "closure capturing a variable from the defining environment"),
            TestCase(R"##(
(let (make-adder (fn (a)
                   (fn (b) (+ a b)))
      make-multi-adder (fn (a b)
                         (let (adder-a (make-adder a)
                               adder-b (make-adder b))
                           (fn (c)
                             (adder-b (adder-a c)))))
       adder-2-3 (make-multi-adder 2 3))
  (adder-2-3 4))
)##", "", "9", "multiple levels of closures capturing variables from their defining environments"),
            TestCase(R"##(
(do (def countdown
      (fn (x)
        (if (zero? x)
          (print "Done.\n")
          (do (print x)
              (print "...\n")
              (countdown (- x 1))))))
    (countdown 3))
)##", "3...\n2...\n1...\nDone.\n", "nil", "recursive function definition and invocation"),
            TestCase(R"##(
((fn (x y)
  (if (zero? x)
    (print "Done.\n")
    (do (print x)
        (print " ")
        (print y)
        (print "...\n")
        (recur (- x 1)
               (+ y 1)))))
  3 3)
)##", "3 3...\n2 4...\n1 5...\nDone.\n", "nil", "recursive anonymous function with recur"),
            TestCase(R"##(
(loop (a 5)
  (if (not (zero? a))
    (recur (- a 1))
    1))
)##", "", "1", "recur inside if.then"),
            TestCase(R"##(
(loop (a 5)
  (if (zero? a)
    1
    (recur (- a 1))))
)##", "", "1", "recur inside if.else"),
            TestCase(R"##(
(loop (a 5)
  (if (not (zero? a))
    (recur (- a 1))))
)##", "", "nil", "recur inside if.then, missing else branch"),
            TestCase(R"##(
(loop (a 5)
  (if (not (zero? a))
    (let (new-a (- a 1))
      (recur new-a))
    1))
)##", "", "1", "recur inside if.then > let"),
            TestCase(R"##(
(loop (a 1 b 1)
  (if (zero? a)
    1
    (recur (if true
             (- a 1)
             (+ a 1))
           (loop (c 0)
             c))))
)##", "", "1", "recur with complex arguments (if and nested loop)"),
            TestCase(R"##(
(let (f (fn (a b a)
          (+ a b a)))
  (f 1 2 3))
)##", "", "8", "function with non-unique parameter names, referencing the last occurrence of the parameter"),
            TestCase(R"##(
(let (a 1
      b 2
      c 3
      f (fn (a) ; a = 4
          (let (b (* 10 a)) ; b = 40
            (+ a b c)))) ; 4 + 40 + 3 = 47
  (f 4))
)##", "", "47", "closure with shadowed enclosed variables, one as a parameter and one as a local var"),
            TestCase(R"##(
(loop (x 1 ; 3 iterations, other uses of this variable are shadowed
       sum 0) ; => 6 + 6 + 6 = 18
  (if (> x 3)
    sum
    (let (sum2 (loop (x 1
                      sum 0) ; 1 + 2 + 3 = 6
                 (if (> x 3)
                   sum
                   (recur (inc x) (+ sum x)))))
      (recur (inc x) (+ sum sum2)))))
)##", "", "18", "nested loops with shadowed variables"),
    };

    test("test eval",
         [&runtime](const TestCase &test_case) {
             return test_eval(runtime, test_case);
         },
         [](const TestCase &test_case) {
             return test_case.description;
         },
         cases
    );
}
