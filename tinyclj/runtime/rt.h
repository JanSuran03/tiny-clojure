#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
const Object *tinyclj_rt_add(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_sub(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_mul(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_div(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_print(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_flush(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_to_edn(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_setmacro(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_list(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_cons(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_next(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_seq(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_list_STAR(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_count(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_first(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_error(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_nil(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_string(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_symbol(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_list(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_function(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_integer(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_double(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_boolean(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_var(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_is_character(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_binary_equal(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_identical(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_apply(const Object *self, size_t argc, const Object **argv);

//Object *tinyclj_rt_read(const Object *self, size_t argc, const Object **argv);
//
//Object *tinyclj_rt_read_string(const Object *self, size_t argc, const Object **argv);
// runtime-dependent functions
const Object *tinyclj_rt_macroexpand(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_macroexpand1(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_eval(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_vars(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_nextID(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_epoch_nanos(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_symbol(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_str(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_double(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_long(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_lt(const Object *self, size_t argc, const Object **argv);
const Object *tinyclj_rt_lte(const Object *self, size_t argc, const Object **argv);
}
