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

const Object *tinyclj_vec_to_list(const std::vector<const Object *> &vec) {
    const Object *ret = empty_list();
    for (ssize_t i = ((ssize_t) vec.size()) - 1; i >= 0; i--) {
        ret = tc_list_cons(vec[i], ret);
    }
    return ret;
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
    return const_cast<Object *>(tinyclj_vec_to_list(std::vector<const Object *>(argv, argv + argc)));
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
}
