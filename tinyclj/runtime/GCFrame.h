#pragma once

#include <vector>

#include "types/Object.h"

class Runtime;

struct GCRootFrame {
    GCRootFrame *m_Prev;
    std::vector<Object *> m_Roots;

    GCRootFrame(GCRootFrame *prev);
};

class RootFrameGuard {
    Runtime &m_Runtime;
    GCRootFrame m_Frame = GCRootFrame(nullptr);
public:
    RootFrameGuard(Runtime &runtime,
                   std::vector<Object *> roots);

    ~RootFrameGuard();
};
