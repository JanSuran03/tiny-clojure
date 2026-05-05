#include <iostream>
#include <optional>

#include "types/TCBoolean.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "runtime/Runtime.h"

using Error = std::optional<std::string>;

Error expect_parse_error(const std::string &input)
try {
    const Object *result = Runtime::readString(input);
    std::string result_str = static_cast<const TCString *>(tc_object_to_string(result)->m_Data)->m_Value;
    return "Expected parsing to fail, but it succeeded, result: " + result_str;
} catch (const std::exception &e) {
    return std::nullopt; // No error, parsing failed as expected
}

Error test_parse(const std::string &input, const Object *expected)
try {
    const Object *result = Runtime::readString(input);
    if (tc_object_equals(result, expected) != tc_boolean_const_true) {
        return "Parsed result does not match expected value";
    } else {
        return std::nullopt; // No error
    }
} catch (const std::exception &e) {
    return std::string("Exception during parsing: ") + e.what();
}

struct TestCase {
    std::string input;
    const Object *expected;
    std::string description;
};

bool run_cases(const std::vector<TestCase> &cases) {
    int num_passed = 0;
    for (const auto &test_case: cases) {
        Error error = test_parse(test_case.input, test_case.expected);
        if (error) {
            std::cerr << "Test failed: " << test_case.description << "\n"
                      << "Input: " << test_case.input << "\n"
                      << "Error: " << *error << "\n\n";
        } else {
            num_passed++;
        }
    }
    std::cout << num_passed << " out of " << cases.size() << " tests passed.\n";
    return num_passed == cases.size();
}

struct ErrorTestCase {
    std::string input;
    std::string description;
};

bool run_error_cases(const std::vector<ErrorTestCase> &cases) {
    int num_passed = 0;
    for (const auto &test_case: cases) {
        Error error = expect_parse_error(test_case.input);
        if (error) {
            std::cerr << "Test failed: " << test_case.description << "\n"
                      << "Input: " << test_case.input << "\n"
                      << "Error: " << *error << "\n\n";
        } else {
            num_passed++;
            std::cout << "Test passed: " << test_case.description << std::endl;
        }
    }
    std::cout << num_passed << " out of " << cases.size() << " error tests passed.\n";
    return num_passed == cases.size();
}

int main() {
    Runtime::init();

    std::vector<TestCase> test_cases = {
            {.input = "42", .expected = tc_integer_new(42), .description = "Parse integer literal"},
            {.input = "-42", .expected = tc_integer_new(-42), .description = "Parse negative integer literal"},
            {.input = "3.14", .expected = tc_double_new(3.14), .description = "Parse float literal"},
            {.input = "-3.14", .expected = tc_double_new(-3.14), .description = "Parse negative float literal"},
            {.input = "true", .expected = tc_boolean_const_true, .description = "Parse boolean true"},
            {.input = "false", .expected = tc_boolean_const_false, .description = "Parse boolean false"},
            {.input = "nil", .expected = nullptr, .description = "Parse nil literal"},
            {.input = "\"Hello\"", .expected = tc_string_new("Hello"), .description = "Parse string literal"},
            {.input = R"("Hello\n\t\r\b\"\\world")",
                    .expected = tc_string_new("Hello\n\t\r\b\"\\world"),
                    .description = "Parse string literal with escape sequences"},
            {.input = "foo", .expected = tc_symbol_new("foo"), .description = "Parse symbol"},
            {.input = "--42",
                    .expected = tc_symbol_new(
                            "--42"), .description = "Parse symbol that looks like a number"},
            {.input = "(1 \"hello\" (true false))",
                    .expected = tc_list_create3(
                            tc_integer_new(1),
                            tc_string_new("hello"),
                            tc_list_create2(tc_boolean_const_true, tc_boolean_const_false)),}
    };

    std::vector<ErrorTestCase> error_cases = {
            {.input = "\"Unterminated string", .description = "Parse unterminated string literal"},
            {.input = R"("Invalid escape \x")", .description = "Parse string with invalid escape sequence"},
            {.input = "123abc", .description = "Parse invalid number literal"},
            {.input = "(1 2", .description = "Parse list with missing closing parenthesis"},
            {.input = "3.14.15", .description = "Parse invalid float with multiple dots"},
            {.input = "(+ 1 {2 3})", .description = "Parse invalid list with unexpected character"},
            {.input = "`", .description = "EOF while parsing syntax quote"},
            {.input = "~", .description = "EOF while parsing unquote"},
            {.input = "~@", .description = "EOF while parsing unquote-splicing"},
    };

    if (run_cases(test_cases) && run_error_cases(error_cases)) {
        std::cout << "All parse tests passed successfully!\n";
        return 0;
    } else {
        std::cerr << "Some parse tests failed.\n";
        return 1;
    }
}