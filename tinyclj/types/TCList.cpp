#include "TCList.h"
#include <stdexcept>

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

const Object *tc_list_cons(const Object *head,const Object *tail) {
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

const Object *tc_list_next(const Object *list) {
    if (list == nullptr) {
        return nullptr;
    }
    if (list->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get next of non-list type");
    }
    TCList *list_data = static_cast<TCList *>(list->m_Data);
    if (list_data && list_data->m_Length == 0) {
        return nullptr;
    }
    return list_data->m_Tail;
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
}
