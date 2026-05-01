#include <cstring>
#include <vector>

#include "LispReader.h"
#include "runtime/Runtime.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

struct ReaderEnv {
    // TODO: NESTED SYNTAX QUOTES PROBABLY DON'T BEHAVE CONSISTENTLY!
    std::unordered_map<std::string, std::string> gensym_table;
};

// for the love of God, I really should remove the consts everywhere
const Object *delimiter_object();

bool is_whitespace(int c);

const Object *read_list(BufferedReader &rdr, ReaderEnv &env);

const Object *read_string(BufferedReader &rdr);

const Object *read_number(BufferedReader &rdr);

const Object *read_symbol(BufferedReader &rdr);

std::string readToken(BufferedReader &rdr);

bool is_symbol_char(char c);

const Object *read(BufferedReader &rdr, char closingDelimiter, ReaderEnv &env);

const Object *LispReader::eof_object() {
    static Object eof_obj = Object::createStaticObject(ObjectType::SYMBOL, new TCSymbol{.m_Name = strdup("<EOF>")},
                                                       nullptr);
    return &eof_obj;
}

const Object *delimiter_object() {
    static Object delim_obj = Object::createStaticObject(ObjectType::SYMBOL, new TCSymbol{.m_Name = strdup("<DELIM>")},
                                                         nullptr);
    return &delim_obj;
}

bool is_whitespace(int c) {
    return c == ',' || isspace(c);
}

const Object *read_list(BufferedReader &rdr, ReaderEnv &env) {
    rdr.read(); // consume '('
    std::vector<const Object *> elements;
    while (true) {
        const Object *elem = read(rdr, ')', env);

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
        return tc_boolean_const_true;
    } else if (token == "false") {
        return tc_boolean_const_false;
    } else {
        return tc_symbol_new(token.c_str());
    }
}

const Object *read_character(BufferedReader &rdr) {
    rdr.read(); // consume '\\'
    std::string token = readToken(rdr);
    if (token.empty()) {
        throw std::runtime_error("Error: Expected character literal after '\\'");
    } else if (token.size() == 1) {
        return tc_char_new(token[0]);
    } else if (token == "newline") {
        return tc_char_new('\n');
    } else if (token == "space") {
        return tc_char_new(' ');
    } else if (token == "tab") {
        return tc_char_new('\t');
    } else if (token == "return") {
        return tc_char_new('\r');
    } else if (token == "backspace") {
        return tc_char_new('\b');
    } else {
        throw std::runtime_error("Error: Invalid character literal: " + token);
    }
}

bool is_symbol_char(char c) {
    // todo: for now, allow '/' in all symbols
    // todo: '#' maybe should not appear as the first character
    return isalnum((unsigned char) c) || strchr("!$%&*:<=>?_+-/#", c) != nullptr;
}

const Object *sym_unquote() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("unquote")},
            nullptr);
    return &sym_obj;
}

const Object *sym_unquote_splicing() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("unquote-splicing")},
            nullptr);
    return &sym_obj;
}

const Object *sym_quote() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("quote")},
            nullptr);
    return &sym_obj;
}

const Object *sym_list() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("list")},
            nullptr);
    return &sym_obj;
}

const Object *sym_seq() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("seq")},
            nullptr);
    return &sym_obj;
}

const Object *sym_concat() {
    static Object sym_obj = Object::createStaticObject(
            ObjectType::SYMBOL,
            new TCSymbol{.m_Name = strdup("concat")},
            nullptr);
    return &sym_obj;
}

bool is_unquote_form(const Object *obj) {
    return obj
           && obj->m_Type == ObjectType::LIST
           && static_cast<TCInteger *>(tc_list_length(obj)->m_Data)->m_Value == 2
           && tc_list_first(obj) == sym_unquote();
}

bool is_unquote_splicing_form(const Object *obj) {
    return obj
           && obj->m_Type == ObjectType::LIST
           && static_cast<TCInteger *>(tc_list_length(obj)->m_Data)->m_Value == 2
           && tc_list_first(obj) == sym_unquote_splicing();
}

const Object *read_unquote(BufferedReader &rdr, ReaderEnv &env) {
    rdr.read(); // consume '~'
    int next_c = rdr.peek();
    if (next_c == '@') {
        rdr.read(); // consume '@'
        const Object *obj = read(rdr, 0, env);
        // Create the unquote-splicing form: (unquote-splicing obj)
        return tc_list_create2(sym_unquote_splicing(), obj);
    } else {
        const Object *obj = read(rdr, 0, env);
        // Create the unquote form: (unquote obj)
        return tc_list_create2(sym_unquote(), obj);
    }
}

const Object *process_syntax_quote(const Object *form, ReaderEnv &env) {
    if (form == nullptr) {
        return nullptr;
    }

    if (SemanticAnalyzer::isSpecial(form)) {
        // return the quoted special form: (quote form)
        return tc_list_create2(sym_quote(), form);
    } else if (is_unquote_form(form)) {
        // return the unquoted form: form
        return tc_list_second(form);
    } else if (form->m_Type == ObjectType::SYMBOL) {
        // resolve the symbol in context or bind it to a unique symbol in case it ends with '#'
        const TCSymbol *sym = static_cast<const TCSymbol *>(form->m_Data);
        const Object *ret_sym = form;
        std::string sym_name(sym->m_Name);
        if (sym_name.empty()) {
            throw std::runtime_error("Error: Empty symbol name");
        }
        if (sym_name.back() == '#') { // substitute with a gensym
            // gensym
            if (auto it = env.gensym_table.find(sym_name); it != env.gensym_table.end()) {
                ret_sym = tc_symbol_new(it->second.c_str());
            } else {
                size_t id = Runtime::nextId();
                // strip the trailing '#' for the gensym name
                std::string gensym_name = sym_name.substr(0, sym_name.size() - 1);
                gensym_name += "_" + std::to_string(id) + "_auto_";
                env.gensym_table[sym_name] = gensym_name;
                ret_sym = tc_symbol_new(gensym_name.c_str());
            }
        } else {
            // todo: fully qualify the symbol (doesn't do anything currently since there aren't any namespaces yet)
        }
        return tc_list_create2(sym_quote(), ret_sym);
    } else if (form->m_Type == ObjectType::LIST) {
        // recursively expand the list into this form
        const TCList *list = static_cast<const TCList *>(form->m_Data);
        if (list->m_Length == 0) {
            return tc_list_create1(sym_list());
        } else {
            // every inner element returns a sequence of forms that need to be concatenated together
            // to allow unquote-splicing:
            // - simple forms, such as symbols or unquotes, return a single-element list
            // - unquote-splicing forms return the inner form directly for splicing
            // nested syntax-quote forms are expanded recursively
            // `(a ~b ~@c d) => (concat (list (quote a)) (list b) c (list (quote d)))
            std::vector<const Object *> expanded_concat_builder = {sym_concat()};
            for (const Object *l = tc_list_seq(form); l; l = tc_list_next(l)) {
                const Object *elem = tc_list_first(l);
                if (is_unquote_form(elem)) {
                    // (unquote elem) => (list elem)
                    expanded_concat_builder.emplace_back(tc_list_create2(sym_list(), tc_list_second(elem)));
                } else if (is_unquote_splicing_form(elem)) {
                    // (unquote-splicing elem) => elem
                    expanded_concat_builder.emplace_back(tc_list_second(elem));
                } else {
                    expanded_concat_builder.emplace_back(tc_list_create2(sym_list(), process_syntax_quote(elem, env)));
                }
            }
            const Object *expanded_concat = tc_list_from_array(
                    expanded_concat_builder.size(),
                    expanded_concat_builder.data());
            return tc_list_create2(sym_seq(), expanded_concat);
        }
    } else {
        // for other types (numbers, strings, etc), return as is
        return form;
    }
}

const Object *read(BufferedReader &rdr, char closingDelimiter, ReaderEnv &env) {
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
        return read_list(rdr, env);
    } else if (c == '"') {
        return read_string(rdr);
    } else if (c == ';') {
        // comment, skip until end of line
        while (c != '\n' && !rdr.eof()) {
            rdr.read();
            c = rdr.peek();
        }
        return read(rdr, closingDelimiter, env); // read the next form after the comment
    } else if (c == '`') {
        // syntax-quote reader
        rdr.read(); // consume '`'
        const Object *obj = read(rdr, 0, env);
        return process_syntax_quote(obj, env);
    } else if (c == '~') {
        // unquote reader
        return read_unquote(rdr, env);
    } else if (isdigit(c)) {
        return read_number(rdr);
    } else if (c == '\'') {
        rdr.read(); // consume '\''
        const Object *obj = read(rdr, 0, env);
        // Create a list (quote obj)
        return tc_list_create2(sym_quote(), obj);
    } else if (c == '-') {
        rdr.read(); // consume '-'
        int next_c = rdr.peek();
        rdr.unread(); // put back '-'
        if (isdigit(next_c)) {
            return read_number(rdr);
        } else {
            return read_symbol(rdr);
        }
    } else if (c == '\\') {
        return read_character(rdr);
    } else if (is_symbol_char((char) c)) {
        return read_symbol(rdr);
    } else {
        throw std::runtime_error(std::string("Unexpected character while reading: ") + (char) c);
    }
}

const Object *LispReader::read(BufferedReader &rdr) {
    ReaderEnv env;
    return read(rdr, '\0', env);
}
