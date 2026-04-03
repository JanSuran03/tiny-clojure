#include "GCFrame.h"
#include "Runtime.h"

GCRootFrame::GCRootFrame(GCRootFrame *prev) : m_Prev(prev) {}

RootFrameGuard::RootFrameGuard(std::vector<const Object *> roots) {
    Runtime &rt = Runtime::getInstance();
    m_Frame.m_Roots = std::move(roots);
    m_Frame.m_Prev = rt.m_RootStack;
    rt.m_RootStack = &m_Frame;
}

RootFrameGuard::~RootFrameGuard() {

    Runtime::getInstance().m_RootStack = m_Frame.m_Prev;
}
