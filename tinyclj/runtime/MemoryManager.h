#pragma once

#include <vector>

#include "types/Object.h"

class Runtime;

class MemoryManager {
    std::vector<Object *> m_Storage;
    // the capacity can grow with each heap iteration
    size_t m_HeapCapacity = 40; // todo: keep it small for testing, increase for production
    // todo: is this needed?
    bool m_GCInProgress = false;

    void markRoots();

    void sweep();

    void destroyObject(Object *obj);

    // marks the object and all objects reachable from it as live (not to be collected)
    void mark(const Object *obj);

public:
    MemoryManager() = default;

    Object *createObject(ObjectType type, void *data, const MethodTable *methodTable, bool isStatic = false);

    void collectGarbage();

    void collectGarbageIfNeeded();
};
