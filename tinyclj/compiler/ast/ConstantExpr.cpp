#include "BooleanExpr.h"
#include "CharExpr.h"
#include "ConstantExpr.h"
#include "DoubleExpr.h"
#include "IntegerExpr.h"
#include "StringExpr.h"
#include "NilExpr.h"
#include "runtime/Runtime.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"

EmitResult emitBoolConstant(const Object *boolObj, CodegenContext &ctx) {
    return BooleanExpr(static_cast<TCBoolean *>(boolObj->m_Data)->m_Value).emitIR(ctx);
}

EmitResult emitCharConstant(const Object *charObj, CodegenContext &ctx) {
    return CharExpr(static_cast<TCChar *>(charObj->m_Data)->m_Value).emitIR(ctx);
}

EmitResult emitDoubleConstant(const Object *doubleObj, CodegenContext &ctx) {
    return DoubleExpr(static_cast<TCDouble *>(doubleObj->m_Data)->m_Value).emitIR(ctx);
}

EmitResult emitIntConstant(const Object *intObj, CodegenContext &ctx) {
    return IntegerExpr(static_cast<TCInteger *>(intObj->m_Data)->m_Value).emitIR(ctx);
}

EmitResult emitStringConstant(const Object *stringObj, CodegenContext &ctx) {
    return StringExpr(static_cast<TCString *>(stringObj->m_Data)->m_Value).emitIR(ctx);
}

EmitResult emitSymbolConstant(const Object *symbolObj, CodegenContext &ctx) {
    using namespace llvm;

    FunctionType *symbol_new_fn_type = FunctionType::get(ctx.pointerType(), /*Object::getObjectStructType(ctx)*/
                                                         {Type::getInt8PtrTy(*ctx.m_LLVMContext)},
                                                         false);
    FunctionCallee symbol_new_fn = ctx.m_Module->getOrInsertFunction("tc_symbol_new", symbol_new_fn_type);
    const char *symbol_name = static_cast<TCSymbol *>(symbolObj->m_Data)->m_Name;
    Value *name_global = ctx.registerGlobalString(symbol_name);
    return ctx.m_IRBuilder.CreateCall(symbol_new_fn, {name_global}, "symbol_const");
}

EmitResult emitListLiteral(const Object *listObj, CodegenContext &ctx) {
    using namespace llvm;

    const TCList *listData = static_cast<TCList *>(listObj->m_Data);
    tc_int_t list_len = static_cast<TCInteger *>(tc_list_length(listObj)->m_Data)->m_Value;

    if (list_len == 0) {
        FunctionType *empty_list_fn_type = FunctionType::get(ctx.pointerType(), {}, false);
        FunctionCallee empty_list_fn = ctx.m_Module->getOrInsertFunction("empty_list", empty_list_fn_type);
        return ctx.m_IRBuilder.CreateCall(empty_list_fn, {}, "empty_list_const");
    }

    Value *list_len_llvm_val = ConstantInt::get(ctx.m_IRBuilder.getInt64Ty(), list_len, false);
    Type *sizeTy = ctx.m_IRBuilder.getIntPtrTy(ctx.m_Module->getDataLayout());

    FunctionType *list_from_array_fn_type = FunctionType::get(ctx.pointerType(),
                                                              {sizeTy, ctx.pointerArrayType()},
                                                              false);
    FunctionCallee list_from_array_fn = ctx.m_Module->getOrInsertFunction("tc_list_from_array",
                                                                          list_from_array_fn_type);

    // todo: optimize with stack coloring:
    // https://llvm.org/doxygen/StackColoring_8cpp_source.html
    // for now, preallocate stack space for argv in the function's entry block
    Function *current_function = ctx.currentFunction();
    IRBuilder<> entry_builder(&current_function->getEntryBlock(), current_function->getEntryBlock().begin());
    AllocaInst *argv = entry_builder.CreateAlloca(ctx.pointerType(),
                                                  list_len_llvm_val,
                                                  "list_const_argv_len_" + std::to_string(list_len));
    size_t i = 0;
    for (; listObj; i++, listObj = tc_list_next(listObj)) {
        const Object *elem = tc_list_first(listObj);
        Value *elem_ptr = entry_builder.CreateGEP(ctx.pointerType(),
                                                  argv,
                                                  ConstantInt::get(sizeTy, i),
                                                  "list_const_elem_ptr_" + std::to_string(i));
        // todo: llvm::ConstantExpr exists -> need ::ConstantExpr to disambiguate
        // (todo: add a custom namespace or write using llvm::FunctionType or something to avoid this)
        EmitResult elem_ir = ::ConstantExpr(elem).emitIR(ctx);
        ctx.m_IRBuilder.CreateStore(elem_ir.value(), elem_ptr);
    }
    return ctx.m_IRBuilder.CreateCall(list_from_array_fn,
                                      {list_len_llvm_val, argv},
                                      "list_const");
}

EmitResult ConstantExpr::emitIR(CodegenContext &ctx) const {
    if (m_Obj == nullptr) {
        return NilExpr().emitIR(ctx);
    }
    switch (m_Obj->m_Type) {
        case ObjectType::BOOLEAN:
            return emitBoolConstant(m_Obj, ctx);
        case ObjectType::CHARACTER:
            return emitCharConstant(m_Obj, ctx);
        case ObjectType::DOUBLE:
            return emitDoubleConstant(m_Obj, ctx);
        case ObjectType::INTEGER:
            return emitIntConstant(m_Obj, ctx);
        case ObjectType::LIST:
            return emitListLiteral(m_Obj, ctx);
        case ObjectType::STRING:
            return emitStringConstant(m_Obj, ctx);
        case ObjectType::SYMBOL:
            return emitSymbolConstant(m_Obj, ctx);
        case ObjectType::CLOSURE:
        case ObjectType::FUNCTION:
        case ObjectType::VAR: // todo: this should not throw, need to emit a reference to the var object
            throw std::runtime_error("Cannot emit as constant: type = "
                                     + std::to_string(static_cast<int>(m_Obj->m_Type)));
    }
    std::unreachable();
}

const Object *ConstantExpr::eval() const {
    return m_Obj;
}

ConstantExpr::ConstantExpr(const Object *quotedValue) : m_Obj(quotedValue) {}

AExpr ConstantExpr::parse(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    form = tc_list_next(form);
    if (!form) {
        throw std::runtime_error("quote requires an argument");
    }
    auto quotedValue = tc_list_first(form);
    if (tc_list_next(form)) {
        throw std::runtime_error("quote takes exactly one argument");
    }
    return std::make_unique<ConstantExpr>(quotedValue);
}
