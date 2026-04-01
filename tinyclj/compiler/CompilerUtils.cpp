#include "CompilerUtils.h"
#include "compiler/ast/NilExpr.h"
#include "runtime/Runtime.h"

EmitResult CompilerUtils::emitBody(const std::vector<AExpr> &body,
                                   const std::string &bodyPrefix,
                                   CodegenContext &ctx) {
    if (body.empty()) {
        return NilExpr().emitIR(ctx);
    }

    EmitResult result;
    for (const auto &i: body) {
        result = i->emitIR(ctx);
    }
    return result;
}

llvm::Value *CompilerUtils::emitObjectPtr(Object *obj, CodegenContext &ctx) {
    using namespace llvm;
    return ctx.m_IRBuilder.CreateIntToPtr(
            ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext), reinterpret_cast<uint64_t>(obj), false),
            ctx.pointerType(),
            "var_ptr");
}
