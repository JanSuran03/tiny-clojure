#include "DefExpr.h"
#include "../../types/TCList.h"

void DefExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("DefExpr::emitIR not implemented");
}

AExpr DefExpr::parse(Object *form) {
    TCList *list = static_cast<TCList *>(form->m_Data);

    // no declaration possible: need form (def name value)
    if (list->m_Length != 3) {
        throw std::runtime_error("def form must have exactly 2 arguments: (def name value)");
    }
    throw std::runtime_error("DefExpr::parse not implemented");
}
