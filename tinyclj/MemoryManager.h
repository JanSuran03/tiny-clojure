#pragma once

#include <vector>

#include "types/Object.h"

struct MemoryManager {
    std::vector<Object *> m_Storage;

    MemoryManager() = default;

    Object *createObject(ObjectType type, void *data, CallFn callFn = nullptr, bool isStatic = false);

    // mark-and-sweep GC (bfs)
    void mark(Object *obj);

    void destroyObject(Object *obj);
};
