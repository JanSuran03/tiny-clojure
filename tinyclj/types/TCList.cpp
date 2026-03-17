#include <stdexcept>

#include "TCList.h"
#include "TCInteger.h"

extern "C" {
const Object *empty_list() {
    static TCList empty = {
            .m_Head = nullptr,
            .m_Tail = nullptr,
            .m_Length = 0
    };
    static Object emptylist_obj = {
            .m_Data = &empty,
            .m_Type = ObjectType::LIST,
            .m_Call = nullptr
    };
    return &emptylist_obj;
}

const Object *tc_list_cons(const Object *head, const Object *tail) {
    if (tail == nullptr) {
        TCList *list = new TCList{
                .m_Head = head,
                .m_Tail = nullptr,
                .m_Length = 1
        };
        return new Object{
                .m_Data = list,
                .m_Type = ObjectType::LIST,
                .m_Call = nullptr
        };
    } else if (tail->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot cons to non-list type");
    } else {
        TCList *tail_list = static_cast<TCList *>(tail->m_Data);
        TCList *list = new TCList{
                .m_Head = head,
                .m_Tail = tail,
                .m_Length = tail_list->m_Length + 1
        };
        return new Object{
                .m_Data = list,
                .m_Type = ObjectType::LIST,
                .m_Call = nullptr
        };
    }
}

const Object *tc_list_first(const Object *list) {
    if (list == nullptr) {
        return nullptr;
    }
    if (list->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get first of non-list type");
    }
    TCList *list_data = static_cast<TCList *>(list->m_Data);
    return list_data->m_Head;
}

const Object *tc_list_next(const Object *obj) {
    if (obj == nullptr) {
        return nullptr;
    }
    if (obj->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get next of non-obj type");
    }
    const Object *ret = static_cast<TCList *>(obj->m_Data)->m_Tail;
    if (ret == nullptr || static_cast<TCList *>(ret->m_Data)->m_Length == 0) {
        return nullptr;
    }
    return ret;
}

const Object *tc_list_seq(const Object *list) {
    if (list == nullptr) {
        return nullptr;
    }
    if (list->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get seq of non-list type");
    }
    if (static_cast<TCList *>(list->m_Data)->m_Length == 0) {
        return nullptr;
    }
    return list;
}

const Object *tc_list_length(const Object *list) {
    if (list == nullptr) {
        return new Object{
                .m_Data = new TCInteger{.m_Value = 0},
                .m_Type = ObjectType::INTEGER,
                .m_Call = nullptr
        };
    }
    if (list->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get length of non-list type");
    }
    TCList *list_data = static_cast<TCList *>(list->m_Data);
    return new Object{
            .m_Data = new TCInteger{.m_Value = list_data->m_Length},
            .m_Type = ObjectType::INTEGER,
            .m_Call = nullptr
    };
}
}
