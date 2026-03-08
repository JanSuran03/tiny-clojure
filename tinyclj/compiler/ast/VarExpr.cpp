#include "VarExpr.h"
#include "types/TCSymbol.h"

void VarExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    if (mode != ExpressionMode::STATEMENT) {
        llvm::AllocaInst *value = ctx.m_VariableMap.at(m_Value);
        ctx.m_IRBuilder.CreateStore(value, dst);
    }
}

VarExpr::VarExpr(std::string value) : m_Value(std::move(value)) {}

AExpr VarExpr::resolveVar(CompilerContext &ctx, const Object *symbol) {
    std::string name = tc_symbol_valueX(symbol);

    if (!ctx.m_AvailableSymbols.contains(name)) {
        throw std::runtime_error(std::string("Cannot resolve symbol: ").append(name).append(" in the context"));
    }
    return std::make_unique<VarExpr>(name);
}
