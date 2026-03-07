#pragma once

#include "../types/Object.h"

extern "C" {
Object *tinyclj_rt_add(Object *arglist);

Object *tinyclj_rt_print(Object *obj);
}
