#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
Object *tinyclj_rt_add(const Object **argv, size_t argc);

Object *tinyclj_rt_print(const Object **argv, size_t argc);

const Object *tinyclj_vec_to_list(const std::vector<const Object *> &vec);
}
