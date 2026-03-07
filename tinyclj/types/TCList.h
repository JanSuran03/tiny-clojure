#pragma once

#include "Object.h"

struct TCList {
    Object *m_Head;
    Object *m_Tail;
    size_t m_Length;
};

extern "C" {
    const Object *empty_list();

    Object *tc_list_cons(Object *head, Object *tail);

    Object *tc_list_first(Object *list);

    Object *tc_list_next(Object *list);

    Object *tc_list_seq(Object *list);

    // todo
    // void tc_list_destroy(TCList *list);
}
