#include <iostream>

#include "rt.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"

extern "C" {
Object *tinyclj_rt_add(const Object *arglist) {
    TCList *list = static_cast<TCList *>(arglist->m_Data);
    if (list == nullptr || list->m_Length != 2) {
        throw std::runtime_error("add function requires exactly 2 arguments");
    }

    const Object *a = list->m_Head;
    list = static_cast<TCList *>(list->m_Tail->m_Data);
    const Object *b = list->m_Head;

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

Object *tinyclj_rt_print(const Object *a) {
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
                std::cout << '"' << static_cast<TCString *>(a->m_Data)->m_Value << '"';
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
                    tinyclj_rt_print(tc_list_first(s));
                }
                std::cout << ')';
                break;
            }
            case ObjectType::CHARACTER:
                std::cout << '\\' << static_cast<TCChar *>(a->m_Data)->m_Value;
                break;
            default:
                std::cout << "<object of type " << static_cast<int>(a->m_Type) << ">";
        }
    }
    return nullptr;
}
}
