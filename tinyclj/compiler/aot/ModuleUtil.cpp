#include "ModuleUtil.h"
#include "util.h"

std::unordered_map<std::string, llvm::GlobalVariable *>
ModuleUtil::declareReferencedGlobals(CodegenContext &ctx, const std::unordered_set<std::string> &global_names) {
    using namespace llvm;
    std::unordered_map<std::string, GlobalVariable *> global_vars;
    for (const std::string &global_name: global_names) {
        GlobalVariable *global_var = new llvm::GlobalVariable(
                *ctx.m_Module,
                ctx.pointerType(),
                false,
                llvm::GlobalValue::PrivateLinkage,
                ConstantPointerNull::get(ctx.pointerType()),
                "var_" + global_name);
        global_vars.emplace(global_name, global_var);
    }
    return global_vars;
}

llvm::Function *ModuleUtil::initReferencedGlobals(
        CodegenContext &ctx,
        const std::string &module_name,
        const std::unordered_map<std::string, llvm::GlobalVariable *> &global_vars) {
    using namespace llvm;
    FunctionType *init_fn_type = FunctionType::get(Type::getVoidTy(*ctx.m_LLVMContext), {}, false);
    // the init function for function modules only needs to initialize global var references
    Function *module_init_fn = Function::Create(init_fn_type,
                                                Function::ExternalLinkage,
                                                util::module_init_fn_name(module_name),
                                                *ctx.m_Module);
    Type *objPtrTy = ctx.pointerType();
    Type *charPtrTy = Type::getInt8PtrTy(*ctx.m_LLVMContext);
    FunctionType *declare_var_fn_type = FunctionType::get(objPtrTy, {charPtrTy}, false);
    Function *declare_var_fn = Function::Create(declare_var_fn_type,
                                                Function::ExternalLinkage,
                                                "tc_runtime_declare_var",
                                                *ctx.m_Module);


    BasicBlock *entry_block = BasicBlock::Create(*ctx.m_LLVMContext, "entry", module_init_fn);
    ctx.m_IRBuilder.SetInsertPoint(entry_block);
    for (const auto &[name, global_var]: global_vars) {
        Value *global_name_global_str = ctx.m_IRBuilder.CreateGlobalStringPtr(
                name, "global_name_str_" + name);
        Value *var_obj = ctx.m_IRBuilder.CreateCall(declare_var_fn, {global_name_global_str}, "var_obj");
        ctx.m_IRBuilder.CreateStore(var_obj, global_var);
    }
    ctx.m_IRBuilder.CreateRetVoid();

    return module_init_fn;
}
