#pragma once

#include <vector>

#include "types/Object.h"

class Runtime;

struct GCRootFrame {
    GCRootFrame *m_Prev;
    std::vector<const Object *> m_Roots;

    GCRootFrame(GCRootFrame *prev);
};

class RootFrameGuard {
    GCRootFrame m_Frame = GCRootFrame(nullptr);
public:
    RootFrameGuard(std::vector<const Object *> roots);

    ~RootFrameGuard();
};
