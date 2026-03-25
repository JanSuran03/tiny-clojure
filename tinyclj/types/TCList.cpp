#include <stdexcept>

#include "runtime/Runtime.h"
#include "TCList.h"
#include "TCInteger.h"

extern "C" {
const Object *empty_list() {
    static TCList empty = {
            .m_Head = nullptr,
            .m_Tail = nullptr,
            .m_Length = 0
    };
    static Object emptylist_obj = Object::createStaticObject(ObjectType::LIST, &empty);
    return &emptylist_obj;
}

const Object *tc_list_cons(const Object *head, const Object *tail) {
    if (tail == nullptr) {
        TCList *list = new TCList{
                .m_Head = head,
                .m_Tail = nullptr,
                .m_Length = 1
        };

        return Runtime::getInstance().createObject(ObjectType::LIST, list);
    } else if (tail->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot cons to non-list type");
    } else {
        TCList *tail_list = static_cast<TCList *>(tail->m_Data);
        TCList *list = new TCList{
                .m_Head = head,
                .m_Tail = tail,
                .m_Length = tail_list->m_Length + 1
        };

        return Runtime::getInstance().createObject(ObjectType::LIST, list);
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

const Object *tc_list_second(const Object *list) {
    return tc_list_first(tc_list_next(list));
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
        TCInteger *zero_data = new TCInteger{.m_Value = 0};

        return Runtime::getInstance().createObject(ObjectType::INTEGER, zero_data);
    }
    if (list->m_Type != ObjectType::LIST) {
        throw std::runtime_error("Cannot get length of non-list type");
    }
    TCList *list_data = static_cast<TCList *>(list->m_Data);

    TCInteger *length_data = new TCInteger{.m_Value = list_data->m_Length};

    return Runtime::getInstance().createObject(ObjectType::INTEGER, length_data);
}

const Object *tc_list_from_array(size_t len, const Object **arr) {
    // todo: is this necessary?
    const Object *ret = empty_list();
    for (ssize_t i = ((ssize_t) len) - 1; i >= 0; i--) {
        ret = tc_list_cons(arr[i], ret);
    }
    return ret;
}

const Object *tc_list_create1(const Object *elem1) {
    return tc_list_cons(elem1, empty_list());
}

const Object *tc_list_create2(const Object *elem1, const Object *elem2) {
    return tc_list_cons(elem1, tc_list_cons(elem2, empty_list()));
}

const Object *tc_list_create3(const Object *elem1, const Object *elem2, const Object *elem3) {
    return tc_list_cons(elem1, tc_list_cons(elem2, tc_list_cons(elem3, empty_list())));
}
}
