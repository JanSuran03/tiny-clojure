#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
const Object *tinyclj_rt_add(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_sub(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_mul(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_div(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_print(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_flush(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_to_edn(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_setmacro(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_list(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_cons(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_next(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_seq(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_list_STAR(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_count(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_first(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_error(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_nil(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_string(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_symbol(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_list(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_function(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_integer(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_double(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_boolean(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_var(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_is_character(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_binary_equal(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_identical(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_apply(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_read(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_read_string(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_slurp(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_spit(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_macroexpand(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_macroexpand1(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_eval(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_vars(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_nextID(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_epoch_nanos(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_symbol(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_str(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_double(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_long(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_lt(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_lte(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_compile_module(const Object *self, unsigned argc, const Object **argv);
const Object *tinyclj_rt_load_module(const Object *self, unsigned argc, const Object **argv);
}
