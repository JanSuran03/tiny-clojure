#include "CompilerUtils.h"
#include "compiler/ast/NilExpr.h"

void CompilerUtils::emitBody(const std::vector<AExpr> &body,
                             const std::string &bodyPrefix,
                             ExpressionMode mode,
                             llvm::AllocaInst *dst,
                             CompilerContext &ctx) {
    if (body.empty()) {
        NilExpr().emitIR(mode, dst, ctx);
        return;
    }

    auto body_id = bodyPrefix + "__" + std::to_string(ctx.nextId());

    for (size_t i = 0; i + 1 < body.size(); i++) {
        ctx.newTmpBasicBlock();
        body[i]->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
    }

    ctx.newTmpBasicBlock();
    body.back()->emitIR(mode, dst, ctx);
}

llvm::Value *CompilerUtils::emitVarPtr(TCVar *var, CompilerContext &ctx) {
    using namespace llvm;
    return ctx.m_IRBuilder.CreateIntToPtr(
            ConstantInt::get(Type::getInt64Ty(ctx.m_LLVMContext), reinterpret_cast<uint64_t>(var), false),
            ctx.pointerType(),
            "var_ptr");
}
