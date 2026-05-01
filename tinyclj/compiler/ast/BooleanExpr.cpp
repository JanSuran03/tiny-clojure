#include "BooleanExpr.h"
#include "types/TCBoolean.h"

EmitResult BooleanExpr::emitIR(CodegenContext &ctx) const {
    // emit a pointer to a static boolean constant object
    static const std::string true_global_name = "tc_boolean_const_true";
    static const std::string false_global_name = "tc_boolean_const_false";
    llvm::GlobalVariable *globalVar = ctx.m_Module->getGlobalVariable(m_Value ? true_global_name : false_global_name);
    if (!globalVar) {
        globalVar = new llvm::GlobalVariable(
                *ctx.m_Module,
                ctx.pointerType(),
                true, // isConstant
                llvm::GlobalValue::ExternalLinkage,
                nullptr, // initializer will be set in TCBoolean::init()
                m_Value ? true_global_name : false_global_name
        );
    }
    return ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), globalVar, m_Value ? "true_const" : "false_const");
}

const Object *BooleanExpr::eval() const {
    return TCBoolean::getStatic(m_Value);
}

BooleanExpr::BooleanExpr(bool value) : m_Value(value) {}
