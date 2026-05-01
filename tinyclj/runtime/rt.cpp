#include <cstring>
#include <iostream>

#include "rt.h"
#include "Runtime.h"
#include "compiler/SemanticAnalyzer.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "types/TCVar.h"

extern "C" {
const Object *tinyclj_rt_add(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_sub(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_mul(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_div(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_print(const Object *self, unsigned argc, const Object **argv) {
    const Object *a = argv[0];
    std::cout << static_cast<TCString *>(tc_object_to_string(a)->m_Data)->m_Value;
    return nullptr;
}

const Object *tinyclj_rt_flush(const Object *self, unsigned argc, const Object **argv) {
    // probably skip argcnt check here to make flushing more efficient
    std::cout << std::flush;
    return nullptr;
}

const Object *tinyclj_rt_to_edn(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("to-edn requires exactly 1 argument");
    }
    const Object *a = argv[0];
    return tc_object_to_edn(a);
}

const Object *tinyclj_rt_setmacro(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("setmacro requires exactly 1 argument");
    }
    // todo: probably needed here?
    Object *var_obj = const_cast<Object *>(argv[0]);
    if (var_obj == nullptr || var_obj->m_Type != ObjectType::VAR) {
        throw std::runtime_error("setmacro requires a var as argument");
    }
    tc_var_set_macroX(var_obj, true);
    return var_obj;
}

const Object *tinyclj_rt_list(const Object *self, unsigned argc, const Object **argv) {
    return tc_list_from_array(argc, argv);
}

const Object *tinyclj_rt_cons(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("cons requires exactly 2 arguments");
    }
    return tc_list_cons(argv[0], argv[1]);
}

const Object *tinyclj_rt_next(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("next requires exactly 1 argument");
    }
    return tc_list_next(argv[0]);
}

const Object *tinyclj_rt_seq(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("seq requires exactly 1 argument");
    }
    return tc_list_seq(argv[0]);
}

const Object *tinyclj_rt_list_STAR(const Object *self, unsigned argc, const Object **argv) {
    if (argc == 0) {
        throw std::runtime_error("list* requires at least 1 argument");
    }

    const Object *result = argv[argc - 1];
    for (ssize_t i = ssize_t(argc) - 2; i >= 0; i--) {
        result = tc_list_cons(argv[i], result);
    }
    return result;
}

const Object *tinyclj_rt_count(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("count requires exactly 1 argument");
    }

    return tc_list_length(argv[0]);
}

const Object *tinyclj_rt_first(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("first requires exactly 1 argument");
    }
    return tc_list_first(argv[0]);
}

const Object *tinyclj_rt_error(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_is_nil(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("nil? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg == nullptr);
}

const Object *tinyclj_rt_is_string(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("string? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::STRING);
}

const Object *tinyclj_rt_is_symbol(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("symbol? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::SYMBOL);
}

const Object *tinyclj_rt_is_list(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("list? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::LIST);
}

const Object *tinyclj_rt_is_function(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("function? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && (arg->m_Type == ObjectType::FUNCTION
                                                   || arg->m_Type == ObjectType::CLOSURE));
}

const Object *tinyclj_rt_is_integer(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("integer? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::INTEGER);
}

const Object *tinyclj_rt_is_double(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("double? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::DOUBLE);
}

const Object *tinyclj_rt_is_boolean(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("boolean? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::BOOLEAN);
}

const Object *tinyclj_rt_is_var(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("var? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::VAR);
}

const Object *tinyclj_rt_is_character(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("character? requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    return TCBoolean::getStatic(arg != nullptr && arg->m_Type == ObjectType::CHARACTER);
}

const Object *tinyclj_rt_binary_equal(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("binary= requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    if (a == b) {
        return TCBoolean::getStatic(true); // same pointer or both nil
    }
    if (a == nullptr || b == nullptr) {
        return TCBoolean::getStatic(false); // one is nil and the other is not
    }
    if (a->m_Type != b->m_Type) {
        return TCBoolean::getStatic(false);
    }
    switch (a->m_Type) {
        case ObjectType::BOOLEAN:
            return TCBoolean::getStatic(static_cast<TCBoolean *>(a->m_Data)->m_Value ==
                                        static_cast<TCBoolean *>(b->m_Data)->m_Value);
        case ObjectType::INTEGER:
            return TCBoolean::getStatic(static_cast<TCInteger *>(a->m_Data)->m_Value ==
                                        static_cast<TCInteger *>(b->m_Data)->m_Value);
        case ObjectType::DOUBLE:
            return TCBoolean::getStatic(static_cast<TCDouble *>(a->m_Data)->m_Value ==
                                        static_cast<TCDouble *>(b->m_Data)->m_Value);
        case ObjectType::STRING:
            return TCBoolean::getStatic(strcmp(static_cast<TCString *>(a->m_Data)->m_Value,
                                               static_cast<TCString *>(b->m_Data)->m_Value) == 0);
        case ObjectType::SYMBOL:
            return TCBoolean::getStatic(strcmp(static_cast<TCSymbol *>(a->m_Data)->m_Name,
                                               static_cast<TCSymbol *>(b->m_Data)->m_Name) == 0);
        case ObjectType::CHARACTER:
            return TCBoolean::getStatic(static_cast<TCChar *>(a->m_Data)->m_Value ==
                                        static_cast<TCChar *>(b->m_Data)->m_Value);
        case ObjectType::LIST: {
            const Object *a_seq = tc_list_seq(a);
            const Object *b_seq = tc_list_seq(b);
            if (static_cast<TCList *>(a->m_Data)->m_Length != static_cast<TCList *>(b->m_Data)->m_Length) {
                return TCBoolean::getStatic(false); // different lengths, can't be equal
            }

            while (a_seq && b_seq) {
                const Object *a_first = tc_list_first(a_seq);
                const Object *b_first = tc_list_first(b_seq);
                const Object *arglist[2] = {a_first, b_first};
                const Object *elem_equal = tinyclj_rt_binary_equal(nullptr, 2, arglist);
                if (!static_cast<TCBoolean *>(elem_equal->m_Data)->m_Value) {
                    return TCBoolean::getStatic(false);
                }
                a_seq = tc_list_next(a_seq);
                b_seq = tc_list_next(b_seq);
            }
            return TCBoolean::getStatic(a_seq == nullptr && b_seq == nullptr); // both should be nil at the end
        }
        default:
            return TCBoolean::getStatic(false); // other types could only be equal by pointer identity
    }
}

const Object *tinyclj_rt_identical(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("identical? requires exactly 2 arguments");
    }
    const Object *a = argv[0];
    const Object *b = argv[1];
    return TCBoolean::getStatic(a == b); // identical? is pointer identity
}

const Object *tinyclj_rt_apply(const Object *self, unsigned argc, const Object **argv) {
    if (argc < 2) {
        throw std::runtime_error("apply requires at least 2 arguments");
    }
    const Object *fn = argv[0];
    if (fn == nullptr) {
        throw std::runtime_error("Cannot apply to nil");
    }
    CallFn callFn = fn->m_MethodTable->m_CallFn;
    if (callFn == nullptr) {
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
    const Object *result = callFn(fn, total_args, call_args);
    delete[] call_args;
    return result;
}

const Object *tinyclj_rt_read(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("read requires exactly 0 arguments");
    }
    return Runtime::read();
}

const Object *tinyclj_rt_read_string(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("read-string requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr || arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("read-string requires a string argument");
    }
    const char *input = static_cast<TCString *>(arg->m_Data)->m_Value;
    return Runtime::readString(input);
}

const Object *tinyclj_rt_slurp(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("slurp requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr || arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("slurp requires a string argument");
    }
    const char *filename = static_cast<TCString *>(arg->m_Data)->m_Value;
    return Runtime::slurp(filename);
}

const Object *tinyclj_rt_spit(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 2) {
        throw std::runtime_error("spit requires exactly 2 arguments");
    }
    const Object *filename_arg = argv[0];
    const Object *content_arg = argv[1];
    if (filename_arg == nullptr || filename_arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("spit requires the first argument to be a string");
    }
    if (content_arg == nullptr || content_arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("spit requires the second argument to be a string");
    }
    const char *filename = static_cast<TCString *>(filename_arg->m_Data)->m_Value;
    const char *content = static_cast<TCString *>(content_arg->m_Data)->m_Value;
    Runtime::spit(filename, content);
    return nullptr;
}

const Object *tinyclj_rt_macroexpand(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_macroexpand1(const Object *self, unsigned argc, const Object **argv) {
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

const Object *tinyclj_rt_eval(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("eval requires exactly 1 argument");
    }
    const Object *form = argv[0];
    if (form == nullptr) {
        return nullptr;
    }

    return Runtime::eval(form);
}

const Object *tinyclj_rt_vars(const Object *self, unsigned argc, const Object **argv) {
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
    return list;
}

const Object *tinyclj_rt_nextID(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("gensym requires exactly 0 arguments");
    }
    size_t id = Runtime::nextId();
    return tc_integer_new(tc_int_t(id));
}

const Object *tinyclj_rt_epoch_nanos(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 0) {
        throw std::runtime_error("systime requires exactly 0 arguments");
    }
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
    return tc_integer_new(nanos);
}

const Object *tinyclj_rt_symbol(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("symbol requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("symbol cannot be created from nil");
    }
    switch (arg->m_Type) {
        case ObjectType::SYMBOL:
            return arg;
        case ObjectType::STRING: {
            const char *name = static_cast<TCString *>(arg->m_Data)->m_Value;
            return tc_symbol_new(name);
        }
        default:
            throw std::runtime_error("symbol can only be created from a string or a symbol");
    }
}

const Object *tinyclj_rt_str(const Object *self, unsigned argc, const Object **argv) {
    if (argc == 0) {
        return tc_string_new("");
    }
    std::string result;
    for (size_t i = 0; i < argc; i++) {
        const Object *arg = argv[i];
        result += static_cast<const TCString *>(tc_object_to_string(arg)->m_Data)->m_Value;
    }
    return tc_string_new(result.c_str());
}

const Object *tinyclj_rt_double(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("double requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("Cannot convert nil to double");
    }
    switch (arg->m_Type) {
        case ObjectType::DOUBLE:
            return arg;
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

const Object *tinyclj_rt_long(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("long requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr) {
        throw std::runtime_error("Cannot convert nil to long");
    }
    switch (arg->m_Type) {
        case ObjectType::INTEGER:
            return arg;
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

const Object *tinyclj_rt_lt(const Object *self, unsigned argc, const Object **argv) {
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
                    return TCBoolean::getStatic(static_cast<TCInteger *>(a->m_Data)->m_Value <
                                                static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return TCBoolean::getStatic(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) <
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot compare integer with non-numeric type using <");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER:
                    return TCBoolean::getStatic(static_cast<TCDouble *>(a->m_Data)->m_Value <
                                                static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                case ObjectType::DOUBLE:
                    return TCBoolean::getStatic(static_cast<TCDouble *>(a->m_Data)->m_Value <
                                                static_cast<TCDouble *>(b->m_Data)->m_Value);
                default:
                    throw std::runtime_error("Cannot compare double with non-numeric type using <");
            }
        default:
            throw std::runtime_error("Cannot compare non-numeric types using <");
    }
}

const Object *tinyclj_rt_lte(const Object *self, unsigned argc, const Object **argv) {
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
                    return TCBoolean::getStatic(static_cast<TCInteger *>(a->m_Data)->m_Value <=
                                                static_cast<TCInteger *>(b->m_Data)->m_Value);
                }
                case ObjectType::DOUBLE: {
                    return TCBoolean::getStatic(
                            static_cast<tc_double_t>(static_cast<TCInteger *>(a->m_Data)->m_Value) <=
                            static_cast<TCDouble *>(b->m_Data)->m_Value);
                }
                default:
                    throw std::runtime_error("Cannot compare integer with non-numeric type using <=");
            }
        case ObjectType::DOUBLE:
            switch (b->m_Type) {
                case ObjectType::INTEGER:
                    return TCBoolean::getStatic(static_cast<TCDouble *>(a->m_Data)->m_Value <=
                                                static_cast<tc_double_t>(static_cast<TCInteger *>(b->m_Data)->m_Value));
                case ObjectType::DOUBLE:
                    return TCBoolean::getStatic(static_cast<TCDouble *>(a->m_Data)->m_Value <=
                                                static_cast<TCDouble *>(b->m_Data)->m_Value);
                default:
                    throw std::runtime_error("Cannot compare double with non-numeric type using <=");
            }
        default:
            throw std::runtime_error("Cannot compare non-numeric types using <=");
    }
}

const Object *tinyclj_rt_compile_module(const Object *self, unsigned argc, const Object **argv) {
    if (argc != 1) {
        throw std::runtime_error("compile-file requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr || arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("compile-file requires a string argument");
    }
    const char *moduleName = static_cast<TCString *>(arg->m_Data)->m_Value;
    Runtime::getInstance().getAotEngine().compileModule(moduleName);
    return nullptr;
}


const Object *tinyclj_rt_load_module(const Object *self, unsigned argc, const Object **argv) {
    if (argc < 1 || argc > 2) {
        throw std::runtime_error("load-file requires exactly 1 argument");
    }
    const Object *arg = argv[0];
    if (arg == nullptr || arg->m_Type != ObjectType::STRING) {
        throw std::runtime_error("load-file requires a string argument");
    }
    const char *moduleName = static_cast<TCString *>(arg->m_Data)->m_Value;
    bool force_reload = false;
    if (argc == 2) {
        const Object *forceArg = argv[1];
        if (forceArg == nullptr || forceArg->m_Type != ObjectType::BOOLEAN) {
            throw std::runtime_error("Second argument to load-file must be a boolean");
        }
        force_reload = static_cast<TCBoolean *>(forceArg->m_Data)->m_Value;
    }
    Runtime::getInstance().getAotEngine().loadCompiledModule(moduleName, force_reload);
    return nullptr;
}
} // extern "C"
