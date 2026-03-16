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

    auto body_id = bodyPrefix + "__" + std::to_string(ctx.nextId());

    for (size_t i = 0; i + 1 < body.size(); i++) {
        ctx.newTmpBasicBlock();
        body[i]->emitIR(ExpressionMode::STATEMENT, nullptr, ctx);
    }

    ctx.newTmpBasicBlock();
    body.back()->emitIR(mode, dst, ctx);
}
