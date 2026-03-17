#include "LoopBase.h"

LoopBase::LoopBase(llvm::BasicBlock *label,
                   std::vector<llvm::AllocaInst *> variable_storages)
        : label(label),
          variable_storages(std::move(variable_storages)) {}
