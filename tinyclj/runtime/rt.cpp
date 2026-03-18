#include <cstring>
#include <iostream>

#include "rt.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCFunction.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "types/TCVar.h"

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
                std::cout << static_cast<TCSymbol *>(a->m_Data)->m_Value;
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
            case ObjectType::CHARACTER:
                std::cout << '\\' << static_cast<TCChar *>(a->m_Data)->m_Value;
                break;
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

Object *tinyclj_rt_iszero(const Object *self, size_t argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("Cannot add integer with non-numeric type");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        return tc_boolean_new(false);
    }
    switch (arg->m_Type) {
        case ObjectType::INTEGER:
            return tc_boolean_new(static_cast<TCInteger *>(arg->m_Data)->m_Value == 0);
        case ObjectType::DOUBLE:
            return tc_boolean_new(static_cast<TCDouble *>(arg->m_Data)->m_Value == 0.0);
        default:
            throw std::runtime_error("zero? function requires a numeric argument");
    }
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
    return const_cast<Object *>( tc_list_cons(argv[0], argv[1]));
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
            return tc_boolean_new(strcmp(static_cast<TCSymbol *>(a->m_Data)->m_Value,
                                         static_cast<TCSymbol *>(b->m_Data)->m_Value) == 0);
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
}