#include "VarExpr.h"

void VarExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    // todo: get the variable's memory space...
    throw std::runtime_error("VarExpr::emitIR not implemented yet");
}

VarExpr::VarExpr(std::string value) : m_Value(value) {}
