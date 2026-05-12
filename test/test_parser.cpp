#include <iostream>
#include <optional>

#include "types/TCBoolean.h"
#include "types/TCChar.h"
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
            //std::cout << "Test passed: " << test_case.description << std::endl;
        }
    }
    std::cout << num_passed << " out of " << cases.size() << " error tests passed.\n";
    return num_passed == cases.size();
}

int main() {
    Runtime::init();

    std::vector<TestCase> test_cases = {
            {.input = "42", .expected = tc_integer_new(42), .description = "Read integer literal"},
            {.input = "-42", .expected = tc_integer_new(-42), .description = "Read negative integer literal"},
            {.input = "3.14", .expected = tc_double_new(3.14), .description = "Read float literal"},
            {.input = "-3.14", .expected = tc_double_new(-3.14), .description = "Read negative float literal"},
            {.input = "\\a", .expected = tc_char_new('a'), .description = "Read character literal"},
            {.input = "\\o",
                    .expected = tc_char_new('o'),
                    .description = "Read 'o' character literal - should not be treated as octal"},
            {.input = "\\newline", .expected = tc_char_new('\n'), .description = "Read named character literal"},
            {.input = "\\space", .expected = tc_char_new(' '), .description = "Read named character literal for space"},
            {.input = "\\tab", .expected = tc_char_new('\t'), .description = "Read named character literal for tab"},
            {.input = "\\return",
                    .expected = tc_char_new('\r'),
                    .description = "Read named character literal for return"},
            {.input = "\\backspace",
                    .expected = tc_char_new('\b'),
                    .description = "Read named character literal for backspace"},
            {.input = "\\o101",
                    .expected = tc_char_new('A'), // octal 101 = decimal 65 = 'A'
                    .description = "Read octal character literal"},
            {.input = "\\o0",
                    .expected = tc_char_new('\0'),
                    .description = "Read octal character literal with value 0"},
            {.input = "\\o377",
                    .expected = tc_char_new((char) 255), // octal 377 = decimal 255
                    .description = "Read octal character literal with max value"},
            {.input = "\\x41",
                    .expected = tc_char_new('A'), // hexadecimal 41 = decimal 65 = 'A'
                    .description = "Read hexadecimal character literal"},
            {.input = "\\x00",
                    .expected = tc_char_new('\0'),
                    .description = "Read hexadecimal character literal with value 0"},
            {.input = "\\xFF",
                    .expected = tc_char_new((char) 255), // hexadecimal FF = decimal 255
                    .description = "Read hexadecimal character literal with max value"},
            {.input = "true", .expected = tc_boolean_const_true, .description = "Read boolean true"},
            {.input = "false", .expected = tc_boolean_const_false, .description = "Read boolean false"},
            {.input = "nil", .expected = nullptr, .description = "Read nil literal"},
            {.input = "\"Hello\"", .expected = tc_string_new("Hello"), .description = "Read string literal"},
            {.input = R"("Hello\n\t\r\b\"\\world")",
                    .expected = tc_string_new("Hello\n\t\r\b\"\\world"),
                    .description = "Read string literal with escape sequences"},
            {.input = "foo", .expected = tc_symbol_new("foo"), .description = "Read symbol"},
            {.input = "--42",
                    .expected = tc_symbol_new(
                            "--42"), .description = "Read symbol that looks like a number"},
            {.input = "(1 \"hello\" (true false))",
                    .expected = tc_list_create3(
                            tc_integer_new(1),
                            tc_string_new("hello"),
                            tc_list_create2(tc_boolean_const_true, tc_boolean_const_false)),},
    };

    std::vector<ErrorTestCase> error_cases = {
            {.input = "\\", .description = "Read character literal with missing token"},
            {.input = "\\ a", .description = "Read character literal with space before token"},
            {.input = "\\o8", .description = "Read octal character literal with invalid digit"},
            {.input = "\\o400", // octal 400 = decimal 256, which is out of range for a char
                    .description = "Read octal character literal with value out of range"},
            {.input = "\\o4ab", .description = "Read octal character literal with invalid octal digits"},
            {.input = "\\o-10", .description = "Read octal character literal with negative value"},
            {.input = "\\xG1", .description = "Read hexadecimal character literal with invalid digit"},
            {.input = "\\x100", // hexadecimal 100 = decimal 256, which is out of range for a char
                    .description = "Read hexadecimal character literal with value out of range"},
            {.input = "\\x-1", .description = "Read hexadecimal character literal with negative value"},
            {.input = "\"Unterminated string", .description = "Read unterminated string literal"},
            {.input = R"("Invalid escape \x")", .description = "Read string with invalid escape sequence"},
            {.input = "123abc", .description = "Read invalid number literal"},
            {.input = "(1 2", .description = "Read list with missing closing parenthesis"},
            {.input = "3.14.15", .description = "Read invalid float with multiple dots"},
            {.input = "(+ 1 {2 3})", .description = "Read invalid list with unexpected character"},
            {.input = "`", .description = "EOF while reading syntax quote"},
            {.input = "~", .description = "EOF while reading unquote"},
            {.input = "~@", .description = "EOF while reading unquote-splicing"},
    };

    if (run_cases(test_cases) && run_error_cases(error_cases)) {
        std::cout << "All parse tests passed successfully!\n";
        return 0;
    } else {
        std::cerr << "Some parse tests failed.\n";
        return 1;
    }
}