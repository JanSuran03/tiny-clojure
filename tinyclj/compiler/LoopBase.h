#pragma once

#include <vector>

#include "llvm/IR/IRBuilder.h"

struct LoopBase {
    llvm::BasicBlock *label;
    std::vector<llvm::AllocaInst *> variable_storages;

    LoopBase(llvm::BasicBlock *label, std::vector<llvm::AllocaInst *> variable_storages);
};