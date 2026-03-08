#pragma once

#include "Object.h"

struct TCList {
    const Object *m_Head;
    const Object *m_Tail;
    size_t m_Length;
};

extern "C" {
    const Object *empty_list();

    const Object *tc_list_cons(const Object *head,const Object *tail);

    const Object *tc_list_first(const Object *list);

    const Object *tc_list_next(const Object *list);

    const Object *tc_list_seq(const Object *list);

    // todo
    // void tc_list_destroy(TCList *list);
}
