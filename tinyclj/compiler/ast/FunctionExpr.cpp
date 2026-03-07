#include "FunctionExpr.h"
//#include "../../runtime/Object.h"

FunctionExpr::FunctionExpr(std::string name, std::vector<std::string> args, AExpr body)
        : m_Name(std::move(name)),
          m_Args(std::move(args)),
          m_Body(std::move(body)) {}

void FunctionExpr::emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const {
    throw std::runtime_error("FunctionExpr::emitIR not implemented");
}
