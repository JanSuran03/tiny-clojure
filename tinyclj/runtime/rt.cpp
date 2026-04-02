#include <cstring>
#include <iostream>

#include "rt.h"
#include "Runtime.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCFunction.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "types/TCVar.h"

std::string unary_to_string(const Object *obj) {
    if (obj == nullptr) {
        return "";
    }
    switch (obj->m_Type) {
        case ObjectType::BOOLEAN:
            return static_cast<TCBoolean *>(obj->m_Data)->m_Value ? "true" : "false";
        case ObjectType::INTEGER:
            return std::to_string(static_cast<TCInteger *>(obj->m_Data)->m_Value);
        case ObjectType::DOUBLE:
            return std::to_string(static_cast<TCDouble *>(obj->m_Data)->m_Value);
        case ObjectType::STRING:
            return static_cast<TCString *>(obj->m_Data)->m_Value;
        case ObjectType::SYMBOL:
            return static_cast<TCSymbol *>(obj->m_Data)->m_Name;
        case ObjectType::CHARACTER:
            return std::string{static_cast<TCChar *>(obj->m_Data)->m_Value};
        case ObjectType::LIST: {
            std::string str = "(";
            bool first = true;
            for (const Object *s = tc_list_seq(obj); s; s = tc_list_next(s)) {
                if (first) {
                    first = false;
                } else {
                    str += ' ';
                }
                const Object *list_elem = tc_list_first(s);
                str += unary_to_string(list_elem);
            }
            str += ')';
            return str;
        }
        case ObjectType::FUNCTION:
            return "function '" + std::string(static_cast<const TCFunction *>(obj->m_Data)->m_Name)
                   + " @" + std::to_string((uintptr_t) obj->m_Call);
        case ObjectType::CLOSURE:
            return "closure @" + std::to_string((uintptr_t) obj->m_Call);
        case ObjectType::VAR:
            return "#'" + std::string(static_cast<const TCVar *>(obj->m_Data)->m_Name);
        default:
            return "<object of type " + std::to_string(static_cast<int>(obj->m_Type)) + ">";
    }
}

std::string unary_to_edn(const Object *obj) {
    if (obj == nullptr) {
        return "nil";
    }
    switch (obj->m_Type) {
        case ObjectType::BOOLEAN:
            return static_cast<TCBoolean *>(obj->m_Data)->m_Value ? "true" : "false";
        case ObjectType::INTEGER:
            return std::to_string(static_cast<TCInteger *>(obj->m_Data)->m_Value);
        case ObjectType::DOUBLE:
            return std::to_string(static_cast<TCDouble *>(obj->m_Data)->m_Value);
        case ObjectType::STRING: {
            std::string buf("\"");
            for (const char *c = static_cast<TCString *>(obj->m_Data)->m_Value; *c; c++) {
                if (*c == '\n') {
                    buf.append("\\n");
                } else if (*c == '\t') {
                    buf.append("\\t");
                } else if (*c == '\r') {
                    buf.append("\\r");
                } else if (*c == '\b') {
                    buf.append("\\b");
                } else if (*c == '\"') {
                    buf.append("\\\"");
                } else if (*c == '\\') {
                    buf.append("\\\\");
                } else {
                    buf.push_back(*c);
                }
            }
            return buf.append("\"");
        }
        case ObjectType::SYMBOL:
            return static_cast<TCSymbol *>(obj->m_Data)->m_Name;
        case ObjectType::CHARACTER: {
            char c = static_cast<TCChar *>(obj->m_Data)->m_Value;
            switch (c) {
                case '\n':
                    return "\\newline";
                case ' ':
                    return "\\space";
                case '\t':
                    return "\\tab";
                case '\r':
                    return "\\return";
                case '\b':
                    return "\\backspace";
                default:
                    return std::string{c};
            }
        }
        case ObjectType::LIST: {
            std::string str = "(";
            bool first = true;
            for (const Object *s = tc_list_seq(obj); s; s = tc_list_next(s)) {
                if (first) {
                    first = false;
                } else {
                    str += ' ';
                }
                const Object *list_elem = tc_list_first(s);
                str += unary_to_edn(list_elem);
            }
            str += ')';
            return str;
        }
        case ObjectType::FUNCTION:
            return "function '" + std::string(static_cast<const TCFunction *>(obj->m_Data)->m_Name)
                   + " @" + std::to_string((uintptr_t) obj->m_Call);
        case ObjectType::CLOSURE:
            return "closure @" + std::to_string((uintptr_t) obj->m_Call);
        case ObjectType::VAR:
            return "#'" + std::string(static_cast<const TCVar *>(obj->m_Data)->m_Name);
        default:
            return "<object of type " + std::to_string(static_cast<int>(obj->m_Type)) + ">";
    }
}

extern "C" {
Object *tinyclj_rt_add(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("add function requires exactly 2 arguments");
    }

    const Object *a = argv[0];
    const Object *b = argv[1];

    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_integer_new(
                            static_cast<TCInteger *>(a->m_Data)->m_Value +
                            static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) +
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot add integer with non-numeric type");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value +
                            static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value +
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot add double with non-numeric type");
            }
        default:
            throw std::runtime_error("Cannot add two non-numeric values");
    }
}

Object *tinyclj_rt_sub(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("add function requires exactly 2 arguments");
    }

    const Object *a = argv[0];
    const Object *b = argv[1];

    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_integer_new(
                            static_cast<TCInteger *>(a->m_Data)->m_Value -
                            static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) -
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot subtract integer and non-numeric type");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value -
                            static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value -
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot subtract double and non-numeric type");
            }
        default:
            throw std::runtime_error("Cannot subtract and non-numeric values");
    }
}

Object *tinyclj_rt_mul(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("add function requires exactly 2 arguments");
    }

    const Object *a = argv[0];
    const Object *b = argv[1];

    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_integer_new(
                            static_cast<TCInteger *>(a->m_Data)->m_Value *
                            static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) *
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot multiply integer with non-numeric type");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value *
                            static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value *
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot multiply double with non-numeric type");
            }
        default:
            throw std::runtime_error("Cannot multiply two non-numeric values");
    }
}

Object *tinyclj_rt_div(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("add function requires exactly 2 arguments");
    }

    const Object *a = argv[0];
    const Object *b = argv[1];

    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    tc_int_t denom = static_cast<TCInteger *>(b->m_Data)->m_Value;
                    if (denom == 0) {
                        throw std::runtime_error("Division by zero");
                    }
                    return tc_integer_new(
                            static_cast<TCInteger *>(a->m_Data)->m_Value /
                            denom);
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) /
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot divide integer by non-numeric type");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value /
                            static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                }
                case ObjectType::DOUBLE: {
                    return tc_double_new(
                            static_cast<TCDouble *>(a->m_Data)->m_Value /
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot divide double by non-numeric type");
            }
        default:
            throw std::runtime_error("Cannot divide two non-numeric values");
    }
}

Object *tinyclj_rt_print(const Object *self, size_t argc, const Object **argv) {
    const Object *a = argv[0];
    if (a == nullptr) {
        std::cout << "nil"; // todo: "nil" or ""?
    } else {
        switch (a->m_Type) {
            case ObjectType::BOOLEAN:
                std::cout << (static_cast<TCBoolean *>(a->m_Data)->m_Value ? "true" : "false");
                break;
            case ObjectType::INTEGER:
                std::cout << static_cast<TCInteger *>(a->m_Data)->m_Value;
                break;
            case ObjectType::DOUBLE:
                std::cout << static_cast<TCDouble *>(a->m_Data)->m_Value;
                break;
            case ObjectType::STRING:
                // todo: edn print vs REPL print
                //   - edn print should escape special characters and wrap in quotes
                //   - REPL print should be more human-friendly and not escape special characters or wrap in quotes
                std::cout/* << '"'*/ << static_cast<TCString *>(a->m_Data)->m_Value/* << '"'*/;
                break;
            case ObjectType::SYMBOL:
                std::cout << static_cast<TCSymbol *>(a->m_Data)->m_Name;
                break;
            case ObjectType::LIST: {
                std::cout << '(';
                bool first = true;
                for (const Object *s = tc_list_seq(a); s; s = tc_list_next(s)) {
                    if (first) {
                        first = false;
                    } else {
                        std::cout << ' ';
                    }
                    const Object *list_elem = tc_list_first(s);
                    tinyclj_rt_print(list_elem, 1, &list_elem); // todo: this is ugly
                }
                std::cout << ')';
                break;
            }
            case ObjectType::CHARACTER: {
                char c = static_cast<TCChar *>(a->m_Data)->m_Value;
                switch (c) {
                    case '\n':
                        std::cout << "\\newline";
                        break;
                    case ' ':
                        std::cout << "\\space";
                        break;
                    case '\t':
                        std::cout << "\\tab";
                        break;
                    case '\r':
                        std::cout << "\\return";
                        break;
                    case '\b':
                        std::cout << "\\backspace";
                        break;
                    default:
                        std::cout << '\\' << c;
                }
                break;
            }
            case ObjectType::FUNCTION:
                std::cout << "Function '"
                          << static_cast<TCFunction *>(a->m_Data)->m_Name
                          << " @" << ((void *) (a->m_Call));
                break;
            case ObjectType::CLOSURE:
                std::cout << "Closure @" << ((void *) (a->m_Call));
                break;
            case ObjectType::VAR:
                std::cout << "#'" << static_cast<TCVar *>(a->m_Data)->m_Name;
                break;
            default:
                std::cout << "<object of type " << static_cast<int>(a->m_Type) << ">";
        }
    }
    return nullptr;
}

Object *tinyclj_rt_flush(const Object *self, size_t argc, const Object **argv) {
    // probably skip argcnt check here to make flushing more efficient
    std::cout << std::flush;
    return nullptr;
}

Object *tinyclj_rt_to_edn(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("to-edn requires exactly 1 argument");
    }
    const Object *a = argv[0];
    std::string edn_str = unary_to_edn(a);
    return tc_string_new(edn_str.c_str());
}

Object *tinyclj_rt_setmacro(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("setmacro requires exactly 1 argument");
    }
    Object *var_obj = const_cast<Object *>(argv[0]);
    if (var_obj == nullptr || var_obj->m_Type != ObjectType::VAR) {
        throw std::runtime_error("setmacro requires a var as argument");
    }
    tc_var_set_macroX(var_obj, true);
    return var_obj;
}

Object *tinyclj_rt_list(const Object *self, size_t argc, const Object **argv) {
    return const_cast<Object *>(tc_list_from_array(argc, argv));
}

Object *tinyclj_rt_cons(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("cons requires exactly 2 arguments");
    }
    return const_cast<Object *>(tc_list_cons(argv[0], argv[1]));
}

Object *tinyclj_rt_next(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("next requires exactly 1 argument");
    }
    return const_cast<Object *>(tc_list_next(argv[0]));
}

Object *tinyclj_rt_seq(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("seq requires exactly 1 argument");
    }
    return const_cast<Object *>(tc_list_seq(argv[0]));
}

Object *tinyclj_rt_list_STAR(const Object *self, size_t argc, const Object **argv) {
    if (argc == 0) {
        throw std::runtime_error("list* requires at least 1 argument");
    }

    const Object *result = argv[argc - 1];
    for (ssize_t i = ssize_t(argc) - 2; i >= 0; i--) {
        result = tc_list_cons(argv[i], result);
    }
    return const_cast<Object *>(result);
}

Object *tinyclj_rt_count(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("count requires exactly 1 argument");
    }

    return const_cast<Object *>(tc_list_length(argv[0]));
}

Object *tinyclj_rt_first(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("first requires exactly 1 argument");
    }
    return const_cast<Object *>(tc_list_first(argv[0]));
}

Object *tinyclj_rt_error(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("error requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr || arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("error requires a string argument");
    }
    const char *message = static_cast<TCString *>(arg->m_Data)->m_Value;
    throw std::runtime_error(message);
}

Object *tinyclj_rt_is_nil(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("nil? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg == nullptr);
}

Object *tinyclj_rt_is_string(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("string? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::STRING);
}

Object *tinyclj_rt_is_symbol(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("symbol? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::SYMBOL);
}

Object *tinyclj_rt_is_list(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("list? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::LIST);
}

Object *tinyclj_rt_is_function(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("function? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && (arg->m_Type == ObjectType::FUNCTION
                                             || arg->m_Type == ObjectType::CLOSURE));
}

Object *tinyclj_rt_is_integer(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("integer? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::INTEGER);
}

Object *tinyclj_rt_is_double(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("double? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::DOUBLE);
}

Object *tinyclj_rt_is_boolean(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("boolean? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::BOOLEAN);
}

Object *tinyclj_rt_is_var(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("var? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::VAR);
}

Object *tinyclj_rt_is_character(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("character? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return tc_boolean_new(arg != nullptr && arg->m_Type == ObjectType::CHARACTER);
}

Object *tinyclj_rt_binary_equal(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("binary= requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    if (a == b) {
        return tc_boolean_new(true); // same pointer or both nil
    }
    if (a == nullptr || b == nullptr) {
        return tc_boolean_new(false); // one is nil and the other is not
    }
    if (a->m_Type != b->m_Type) {
        return tc_boolean_new(false);
    }
    switch (a->m_Type) {
        case ObjectType::BOOLEAN:
            return tc_boolean_new(static_cast<TCBoolean *>(a->m_Data)->m_Value ==
                                  static_cast<TCBoolean *>(b->m_Data)->m_Value);
        case ObjectType::INTEGER:
            return tc_boolean_new(static_cast<TCInteger *>(a->m_Data)->m_Value ==
                                  static_cast<TCInteger *>(b->m_Data)->m_Value);
        case ObjectType::DOUBLE:
            return tc_boolean_new(static_cast<TCDouble *>(a->m_Data)->m_Value ==
                                  static_cast<TCDouble *>(b->m_Data)->m_Value);
        case ObjectType::STRING:
            return tc_boolean_new(strcmp(static_cast<TCString *>(a->m_Data)->m_Value,
                                         static_cast<TCString *>(b->m_Data)->m_Value) == 0);
        case ObjectType::SYMBOL:
            return tc_boolean_new(strcmp(static_cast<TCSymbol *>(a->m_Data)->m_Name,
                                         static_cast<TCSymbol *>(b->m_Data)->m_Name) == 0);
        case ObjectType::CHARACTER:
            return tc_boolean_new(static_cast<TCChar *>(a->m_Data)->m_Value ==
                                  static_cast<TCChar *>(b->m_Data)->m_Value);
        case ObjectType::LIST: {
            const Object *a_seq = tc_list_seq(a);
            const Object *b_seq = tc_list_seq(b);
            if (static_cast<TCList *>(a->m_Data)->m_Length != static_cast<TCList *>(b->m_Data)->m_Length) {
                return tc_boolean_new(false); // different lengths, can't be equal
            }

            while (a_seq && b_seq) {
                const Object *a_first = tc_list_first(a_seq);
                const Object *b_first = tc_list_first(b_seq);
                const Object *arglist[2] = {a_first, b_first};
                Object *elem_equal = tinyclj_rt_binary_equal(nullptr, 2, arglist);
                if (!static_cast<TCBoolean *>(elem_equal->m_Data)->m_Value) {
                    return tc_boolean_new(false);
                }
                a_seq = tc_list_next(a_seq);
                b_seq = tc_list_next(b_seq);
            }
            return tc_boolean_new(a_seq == nullptr && b_seq == nullptr); // both should be nil at the end
        }
        default:
            return tc_boolean_new(false); // other types could only be equal by pointer identity
    }
}

Object *tinyclj_rt_identical(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("identical? requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    return tc_boolean_new(a == b); // identical? is pointer identity
}

Object *tinyclj_rt_apply(const Object *self, size_t argc, const Object **argv) {
    if (argc < 2) {
        throw std::runtime_error("apply requires at least 2 arguments");
    }
    const Object *fn = argv[0];
    if (fn == nullptr) {
        throw std::runtime_error("Cannot apply to nil");
    }
    if (fn->m_Call == nullptr) {
        throw std::runtime_error("Target object is not callable");
    }
    const Object *args_list = tc_list_seq(argv[argc - 1]);
    tc_int_t num_list_args = static_cast<TCInteger *>(tc_list_length(args_list)->m_Data)->m_Value;
    tc_int_t total_args = argc + num_list_args - 2; // all args except the function and the list of args to apply
    const Object **call_args = new const Object *[total_args];
    for (size_t i = 0; i < argc - 2; i++) {
        call_args[i] = argv[i + 1];
    }
    for (size_t i = 0; i < num_list_args; i++) {
        call_args[argc - 2 + i] = tc_list_first(args_list);
        args_list = tc_list_next(args_list);
    }
    Object *result = fn->m_Call(fn, total_args, call_args);
    delete[] call_args;
    return result;
}

Object *tinyclj_rt_macroexpand(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("macroexpand requires exactly 1 argument");
    }
    const Object *form = argv[0];
    if (form == nullptr) {
        return nullptr;
    }

    Runtime &rt = Runtime::getInstance();
    return SemanticAnalyzer::macroexpand(rt, form);
}

Object *tinyclj_rt_macroexpand1(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("macroexpand1 requires exactly 1 argument");
    }
    const Object *form = argv[0];
    if (form == nullptr) {
        return nullptr;
    }

    Runtime &rt = Runtime::getInstance();
    return SemanticAnalyzer::macroexpand1(rt, form);
}

Object *tinyclj_rt_eval(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("eval requires exactly 1 argument");
    }
    const Object *form = argv[0];
    if (form == nullptr) {
        return nullptr;
    }

    Runtime &rt = Runtime::getInstance();
    return rt.eval(form);
}

Object *tinyclj_rt_vars(const Object *self, size_t argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("vars requires exactly 0 arguments");
    }

    Runtime &rt = Runtime::getInstance();
    const std::unordered_map<std::string, Object *> &vars = rt.getGlobalVarStorage();
    const Object **var_array = new const Object *[vars.size()];
    size_t i = 0;
    for (const auto &entry: vars) {
        var_array[i++] = entry.second;
    }
    auto list = tc_list_from_array(vars.size(), var_array);
    delete[] var_array;
    return const_cast<Object *>(list);
}

Object *tinyclj_rt_nextID(const Object *self, size_t argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("gensym requires exactly 0 arguments");
    }
    Runtime &rt = Runtime::getInstance();
    size_t id = rt.nextId();
    return tc_integer_new(tc_int_t(id));
}

Object *tinyclj_rt_epoch_nanos(const Object *self, size_t argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("systime requires exactly 0 arguments");
    }
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
    return tc_integer_new(nanos);
}

Object *tinyclj_rt_symbol(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("symbol requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("symbol cannot be created from nil");
    }
    switch (arg->m_Type) {
        case ObjectType::SYMBOL:
            return const_cast<Object *>(arg); // symbol of a symbol is itself
        case ObjectType::STRING: {
            const char *name = static_cast<TCString *>(arg->m_Data)->m_Value;
            return tc_symbol_new(name);
        }
        default:
            throw std::runtime_error("symbol can only be created from a string or a symbol");
    }
}

Object *tinyclj_rt_str(const Object *self, size_t argc, const Object **argv) {
    if (argc == 0) {
        return tc_string_new("");
    }
    std::string result;
    for (size_t i = 0; i < argc; i++) {
        const Object *arg = argv[i];
        result += unary_to_string(arg);
    }
    return tc_string_new(result.c_str());
}

Object *tinyclj_rt_double(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("double requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("Cannot convert nil to double");
    }
    switch (arg->m_Type) {
        case ObjectType::DOUBLE:
            return const_cast<Object *>(arg); // double of a double is itself
        case ObjectType::INTEGER: {
            tc_double_t value = static_cast<tc_double_t>(static_cast<TCInteger *>(arg->m_Data)->m_Value);
            return tc_double_new(value);
        }
        default:
            throw std::runtime_error("cannot cast argument of type "
                                     + std::to_string(static_cast<int>(arg->m_Type))
                                     + " to double");
    }
}

Object *tinyclj_rt_long(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("long requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("Cannot convert nil to long");
    }
    switch (arg->m_Type) {
        case ObjectType::INTEGER:
            return const_cast<Object *>(arg); // long of an integer is itself
        case ObjectType::DOUBLE: {
            tc_double_t double_value = static_cast<TCDouble *>(arg->m_Data)->m_Value;
            if (double_value < static_cast<tc_double_t>(std::numeric_limits<tc_int_t>::min()) ||
                double_value > static_cast<tc_double_t>(std::numeric_limits<tc_int_t>::max())) {
                throw std::runtime_error("Double value " + std::to_string(double_value) + " is out of range for long");
            }
            tc_int_t value = static_cast<tc_int_t>(double_value);
            return tc_integer_new(value);
        }
        default:
            throw std::runtime_error("cannot cast argument of type "
                                     + std::to_string(static_cast<int>(arg->m_Type))
                                     + " to long");
    }
}

Object *tinyclj_rt_lt(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("builtin < requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    if (a == nullptr || b == nullptr) {
        throw std::runtime_error("Cannot compare nil with <");
    }
    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_boolean_new(static_cast<TCInteger *>(a->m_Data)->m_Value <
                                          static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return tc_boolean_new(static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) <
                                          static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot compare integer with non-numeric type using <");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER:
                    return tc_boolean_new(static_cast<TCDouble *>(a->m_Data)->m_Value <
                                          static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                case ObjectType::DOUBLE:
                    return tc_boolean_new(static_cast<TCDouble *>(a->m_Data)->m_Value <
                                          static_cast<TCDouble *>(b->m_Data)->m_Value);
                default:
                    throw std::runtime_error("Cannot compare double with non-numeric type using <");
            }
        default:
            throw std::runtime_error("Cannot compare non-numeric types using <");
    }
}

Object *tinyclj_rt_lte(const Object *self, size_t argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("builtin <= requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    if (a == nullptr || b == nullptr) {
        throw std::runtime_error("Cannot compare nil with <=");
    }
    switch (a->m_Type) {
        case ObjectType::INTEGER:
            switch (b->m_Type) {
                case ObjectType::INTEGER: {
                    return tc_boolean_new(static_cast<TCInteger *>(a->m_Data)->m_Value <=
                                          static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return tc_boolean_new(static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) <=
                                          static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot compare integer with non-numeric type using <=");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER:
                    return tc_boolean_new(static_cast<TCDouble *>(a->m_Data)->m_Value <=
                                          static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                case ObjectType::DOUBLE:
                    return tc_boolean_new(static_cast<TCDouble *>(a->m_Data)->m_Value <=
                                          static_cast<TCDouble *>(b->m_Data)->m_Value);
                default:
                    throw std::runtime_error("Cannot compare double with non-numeric type using <=");
            }
        default:
            throw std::runtime_error("Cannot compare non-numeric types using <=");
    }
}
}