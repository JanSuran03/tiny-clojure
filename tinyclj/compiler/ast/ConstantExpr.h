#include "Expr.h"

class ConstantExpr : public Expr {
public:
    void emitIR(ExpressionMode mode, llvm::AllocaInst *dst, CompilerContext &ctx) const override;

protected:
    virtual llvm::Value *getConstantValue(CompilerContext &ctx) const = 0;
};
