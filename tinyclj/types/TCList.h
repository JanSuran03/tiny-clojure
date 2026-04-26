#pragma once

#include "Object.h"
#include "tcdef.h"

struct TCList {
    const Object *m_Head;
    const Object *m_Tail;
    tc_int_t m_Length;

    static const Object *toString(const Object *self);

    static const Object *toEDN(const Object *self);

    static MethodTable st_MethodTable;
};

extern "C" {
const Object *empty_list();

const Object *tc_list_cons(const Object *head, const Object *tail);

const Object *tc_list_first(const Object *list);

const Object *tc_list_next(const Object *obj);

const Object *tc_list_second(const Object *list);

const Object *tc_list_seq(const Object *list);

const Object *tc_list_length(const Object *list);

const Object *tc_list_from_array(size_t len, const Object **arr);

const Object *tc_list_create1(const Object *elem1);
const Object *tc_list_create2(const Object *elem1, const Object *elem2);
const Object *tc_list_create3(const Object *elem1, const Object *elem2, const Object *elem3);

// todo
// void tc_list_destroy(TCList *list);
}
