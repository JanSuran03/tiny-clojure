#include "GCFrame.h"
#include "Runtime.h"

GCRootFrame::GCRootFrame(GCRootFrame *prev) : m_Prev(prev) {}

RootFrameGuard::RootFrameGuard(Runtime &runtime,
                               std::vector<Object *> roots)
        : m_Runtime(runtime) {
    m_Frame.m_Roots = std::move(roots);
    m_Frame.m_Prev = runtime.m_RootStack;
    runtime.m_RootStack = &m_Frame;
}

RootFrameGuard::~RootFrameGuard() {
    m_Runtime.m_RootStack = m_Frame.m_Prev;
}
