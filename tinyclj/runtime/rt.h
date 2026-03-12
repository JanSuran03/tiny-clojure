#pragma once

#include <vector>

#include "types/Object.h"

extern "C" {
Object *tinyclj_rt_add(const Object *arglist);

Object *tinyclj_rt_print(const Object *obj);

const Object *tinyclj_vec_to_list(const std::vector<const Object *> &vec);
}
