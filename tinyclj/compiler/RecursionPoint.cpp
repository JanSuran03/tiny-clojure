#include "RecursionPoint.h"

RecursionPoint::RecursionPoint(llvm::BasicBlock *label,
                               std::vector<llvm::PHINode *> phiNodes)
        : m_Label(label),
          m_PhiNodes(std::move(phiNodes)) {}
