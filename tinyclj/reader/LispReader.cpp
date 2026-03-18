#include <cstring>
#include <vector>

#include "LispReader.h"
#include "runtime/rt.h"
#include "types/TCBoolean.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
// for the love of God, I really should remove the consts everywhere

const Object *delimiter_object();

bool is_whitespace(int c);

const Object *read_list(BufferedReader &rdr);

const Object *read_string(BufferedReader &rdr);

const Object *read_number(BufferedReader &rdr);

const Object *read_symbol(BufferedReader &rdr);

std::string readToken(BufferedReader &rdr);

bool is_symbol_char(char c);

const Object *read(BufferedReader &rdr, char closingDelimiter);

const Object *LispReader::eof_object() {
    static Object eof_obj;
    return &eof_obj;
}

const Object *delimiter_object() {
    static Object delim_obj;
    return &delim_obj;
}

bool is_whitespace(int c) {
    return c == ',' || isspace(c);
}

const Object *read_list(BufferedReader &rdr) {
    rdr.read(); // consume '('
    std::vector<const Object *> elements;
    while (true) {
        const Object *elem = read(rdr, ')');

        if (elem == LispReader::eof_object()) {
            throw std::runtime_error("Error: Unterminated list");
        } else if (elem == delimiter_object()) {
            break; // end of list
        }

        elements.emplace_back(elem);
    }
    return tc_list_from_array(elements.size(), elements.data());
}

const Object *read_string(BufferedReader &rdr) {
    rdr.read(); // consume '"'
    std::string buffer;
    while (true) {
        int c = rdr.read();
        if (rdr.eof()) {
            throw std::runtime_error("Error: Unterminated string");
        }

        if (c == '"') {
            return tc_string_new(buffer.c_str());
        } else if (c == '\\') {
            c = rdr.read();
            if (rdr.eof()) {
                throw std::runtime_error("Error: EOF while reading string escape");
            }
            switch (c) {
                case 'n':
                    buffer.push_back('\n');
                    break;
                case 't':
                    buffer.push_back('\t');
                    break;
                case 'r':
                    buffer.push_back('\r');
                    break;
                case 'b':
                    buffer.push_back('\b');
                    break;
                case '"':
                    buffer.push_back('"');
                    break;
                case '\\':
                    buffer.push_back('\\');
                    break;
                default:
                    throw std::runtime_error(std::string("Error: Invalid escape sequence in string: \\") + (char) c);
            }
        } else {
            buffer.push_back((char) c);
        }
    }
}

const Object *read_number(BufferedReader &rdr) {
    std::string buffer;
    bool found_dot = false;
    int c = rdr.peek();
    if (c == '-') {
        buffer.push_back((char) c);
        rdr.read();
        c = rdr.peek();
    }
    while (true) {
        if (c == '\0' || is_whitespace((char) c) || c == ')' || c == '(') {
            break;
        } else if (c == '.') {
            if (found_dot) {
                throw std::runtime_error("Error: Invalid number format, multiple dots: " + buffer + ".");
            }
            found_dot = true;
            buffer.push_back((char) c);
            rdr.read();
            c = rdr.peek();
        } else if (isdigit((unsigned char) c)) {
            buffer.push_back((char) c);
            rdr.read();
            c = rdr.peek();
        } else {
            throw std::runtime_error(std::string("Error: Invalid character in number: ") + (char) c);
        }
    }
    if (found_dot) {
        errno = 0; // reset errno before calling strtod
        double value = strtod(buffer.c_str(), nullptr);
        if (errno != 0) {
            throw std::runtime_error("Floating-point overflow while parsing number: " + buffer);
        }
        return tc_double_new(value);
    } else {
        errno = 0; // reset errno before calling strtoll
        tc_int_t value = strtoll(buffer.c_str(), nullptr, 10);
        // check for overflow
        if (errno != 0) {
            throw std::runtime_error("Integer overflow while parsing number: " + buffer);
        }
        return tc_integer_new(value);
    }
}

std::string readToken(BufferedReader &rdr) {
    std::string token;
    while (is_symbol_char((char) rdr.peek())) {
        token.push_back((char) rdr.read());
    }
    return token;
}

const Object *read_symbol(BufferedReader &rdr) {
    std::string token = readToken(rdr);
    if (token == "nil") {
        return nullptr;
    } else if (token == "true") {
        return tc_boolean_new(true);
    } else if (token == "false") {
        return tc_boolean_new(false);
    } else {
        return tc_symbol_new(token.c_str());
    }
}

bool is_symbol_char(char c) {
    return isalnum((unsigned char) c) || strchr("!$%&*:<=>?_+-", c) != nullptr;
}

const Object *read(BufferedReader &rdr, char closingDelimiter) {
    int c = rdr.peek();
    while (is_whitespace(c)) {
        rdr.read();
        c = rdr.peek();
    }
    if (rdr.eof()) {
        return LispReader::eof_object();
    } else if (c == closingDelimiter) {
        rdr.read(); // consume the closing delimiter
        return delimiter_object();
    } else if (c == '(') {
        return read_list(rdr);
    } else if (c == '"') {
        return read_string(rdr);
    } else if (c == ';') {
        // comment, skip until end of line
        while (c != '\n' && !rdr.eof()) {
            rdr.read();
            c = rdr.peek();
        }
        return read(rdr, closingDelimiter); // read the next form after the comment
    } else if (isdigit(c)) {
        return read_number(rdr);
    } else if (c == '\'') {
        rdr.read(); // consume '\''
        const Object *obj = read(rdr, 0);
        // Create a list (quote obj)
        // todo: cache the symbol
        const Object *quote_symbol = tc_symbol_new("quote");
        return tc_list_cons(quote_symbol, tc_list_cons(obj, empty_list()));
    } else if (c == '-') {
        rdr.read(); // consume '-'
        int next_c = rdr.peek();
        rdr.unread(); // put back '-'
        if (isdigit(next_c)) {
            return read_number(rdr);
        } else {
            return read_symbol(rdr);
        }
    } else if (is_symbol_char((char) c)) {
        return read_symbol(rdr);
    } else {
        throw std::runtime_error(std::string("Unexpected character while reading: ") + (char) c);
    }
}

const Object *LispReader::read(BufferedReader &rdr) {
    return read(rdr, '\0');
}
