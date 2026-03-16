#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
Object *tinyclj_rt_add(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_print(const Object *self, size_t argc, const Object **argv);

Object *tinyclj_rt_iszero(const Object *self, size_t argc, const Object **argv);

const Object *tinyclj_vec_to_list(const std::vector<const Object *> &vec);
}
