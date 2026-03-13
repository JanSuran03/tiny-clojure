#pragma once

#include "BufferedReader.h"

#include "types/Object.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"

namespace LispReader {
    const Object *eof_object();

    const Object *read(BufferedReader &rdr);
}
