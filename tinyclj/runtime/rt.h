#pragma once

#include "types/Object.h"

extern "C" {
Object *tinyclj_rt_add(const Object *arglist);

Object *tinyclj_rt_print(const Object *obj);
}
