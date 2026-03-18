#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
Object *tinyclj_rt_add(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_print(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_iszero(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_setmacro(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_list(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_cons(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_next(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_seq(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_count(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_first(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_error(const Object *self, size_t argc, const Object **argv);
}
