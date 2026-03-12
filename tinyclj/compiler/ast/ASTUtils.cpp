#include "ASTUtils.h"
#include "NilExpr.h"

void
AstUtils::emitBody(const std::vector<AExpr> &body,
                   const std::string &bodyPrefix,
                   ExpressionMode mode,
                   llvm::AllocaInst *dst,
                   CompilerContext &ctx) {
    if (body.empty()) {
        NilExpr().emitIR(mode, dst, ctx);
        return;
    }

    auto body_id = bodyPrefix + "__" + std::to_string(ctx.m_IdCounter++);

    for (size_t i = 0; i + 1 < body.size(); i++) {
        //llvm::BasicBlock *statement_block = llvm::BasicBlock::Create(
        //        ctx.m_LLVMContext,
        //        body_id + "__" + std::to_string(i));
        //ctx.m_IRBuilder.CreateBr(statement_block);
        //ctx.m_IRBuilder.SetInsertPoint(statement_block);
        ctx.newBasicBlock();
        body[i]->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
    }

    ctx.newBasicBlock();

    //llvm::BasicBlock *ret_block = llvm::BasicBlock::Create(
    //        ctx.m_LLVMContext,
    //        body_id + "__ret");
    //ctx.m_IRBuilder.CreateBr(ret_block);
    //ctx.m_IRBuilder.SetInsertPoint(ret_block);
    body.back()->emitIR(mode, dst, ctx);
}
