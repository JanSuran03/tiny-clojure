#pragma once

#include <vector>

#include "llvm/IR/IRBuilder.h"

struct RecursionPoint {
    llvm::BasicBlock *m_Label;
    /// one phi node for each loop variable
    std::vector<llvm::PHINode *> m_PhiNodes;

    RecursionPoint(llvm::BasicBlock *label,
                   std::vector<llvm::PHINode *> phiNodes);
};