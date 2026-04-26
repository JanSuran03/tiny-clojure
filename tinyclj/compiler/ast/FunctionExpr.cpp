#include <iostream>
#include <optional>
#include <utility>

#include "FunctionExpr.h"
#include "util.h"
#include "compiler/CodegenContext.h"
#include "compiler/CompilerUtils.h"
#include "compiler/aot/FunctionModule.h"
#include "compiler/aot/ModuleUtil.h"
#include "local-binding/CapturedLocalExpr.h"
#include "runtime/Runtime.h"
#include "runtime/rt.h"
#include "types/TCClosure.h"
#include "types/TCFunction.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

FunctionExpr::FunctionExpr(std::string name,
                           std::unordered_map<size_t, FunctionOverload> overloads,
                           std::optional<FunctionOverload> variadic_overload,
                           Captures captures,
                           std::unordered_set<std::string> referenced_global_names,
                           std::unordered_set<std::string> module_imports)
        : m_Name(std::move(name)),
          m_Overloads(std::move(overloads)),
          m_VariadicOverload(std::move(variadic_overload)),
          m_Captures(std::move(captures)),
          m_ReferencedGlobalNames(std::move(referenced_global_names)),
          m_ModuleImports(std::move(module_imports)) {}

std::string ambiguousOverload(const std::string &fn_name, size_t variadic_fixed_argcnt, size_t other_fixed_argcnt) {
    return "Ambiguous overloads: A function " + fn_name +
           " has a variadic overload with " + std::to_string(variadic_fixed_argcnt) +
           " fixed arguments, but there is another overload with " +
           std::to_string(other_fixed_argcnt) + " fixed arguments";
}

AExpr FunctionExpr::parse(ExpressionMode mode,
                          AnalyzerContext &ctx,
                          const Object *form,
                          const std::optional<std::string> &nameHint) {
    bool is_eval_wrapper = strcmp(static_cast<const TCSymbol *>(tc_list_first(form)->m_Data)->m_Name,
                                  "__eval_fn_wrapper") == 0;
    const Object *original_form = form;
    form = tc_list_next(form); // consume 'fn

    // (fn name (args...) body...)
    // or (fn (args...) body...)
    // todo: make this actually work - is not used at the moment
    const Object *name_sym = tc_list_first(form);
    std::string name;

    // todo: improve the logic
    if (name_sym != nullptr && name_sym->m_Type == ObjectType::SYMBOL) {
        form = tc_list_next(form); // consume the name if it's present
        name = tc_symbol_valueX(name_sym);
    } else if (nameHint.has_value()) {
        name = nameHint.value() + "__" + std::to_string(Runtime::getInstance().nextId());
    } else {
        name = "fn__" + std::to_string(Runtime::getInstance().nextId());
    }

    // (arglist & body) or ((arglist & body) (arglist2 & body2) ...)
    // transform the first form into the second form if necessary
    // before vectors are supported, it is NOT enough to check, whether the 1st form is a vector: (fn [args] body)
    bool is_list_of_overloads = form != nullptr;
    if (is_list_of_overloads) {
        const Object *first_overload_form_or_arglist = tc_list_first(form);
        if (first_overload_form_or_arglist == nullptr || first_overload_form_or_arglist->m_Type != ObjectType::LIST) {
            throw std::runtime_error("missing argument list for a function definition");
        }
        const Object *maybe_first_overload_arglist = tc_list_first(first_overload_form_or_arglist);
        // ObjecType::SYMBOL in case of a single overload, not wrapped inside an outer list
        is_list_of_overloads = maybe_first_overload_arglist && maybe_first_overload_arglist->m_Type == ObjectType::LIST;
    }

    // if not a list of overloads, wrap the single overload into a list
    if (!is_list_of_overloads) {
        form = tc_list_cons(form, empty_list());
    }

    // create a new capture scope for the function - since the environment is shared across
    // overloads, the capture scope needs to be created before parsing the overloads
    // and the capture scope is the union of the captures from all overloads
    ctx.m_CapturesMappingStack.emplace_back();
    ctx.m_ReferencedGlobalNamesStack.emplace_back();
    ctx.m_ModuleImportsStack.emplace_back();
    // if there exists a variadic overload with m_Args = N (i.e. N-1 fixed args), then there can exist
    // a (non-variadic) overload with m_Args <= N-1 (i.e. m_Args < N)
    std::unordered_map<size_t, FunctionOverload> overloads;
    std::optional<FunctionOverload> variadic_overload;
    for (; form; form = tc_list_next(form)) {
        const Object *overload_form = tc_list_first(form);
        if (overload_form == nullptr || overload_form->m_Type != ObjectType::LIST) {
            throw std::runtime_error("fn body must be a list of overloads, each overload is a list");
        }
        FunctionOverload overload = FunctionOverload::parse(ctx, overload_form, is_eval_wrapper);
        size_t overload_argcnt = overload.m_Args.size();
        if (overload.m_IsVariadic) {
            size_t fixed_argcnt = overload_argcnt - 1;
            if (variadic_overload.has_value()) {
                throw std::runtime_error("Ambiguous overloads: Multiple variadic overloads are not allowed");
            }
            // check for overloads with more fixed arguments
            for (const auto &[existing_argcnt, existing_overload]: overloads) {
                if (existing_argcnt > fixed_argcnt) {
                    throw std::runtime_error(ambiguousOverload(name, fixed_argcnt, existing_argcnt));
                }
            }
            variadic_overload = std::move(overload);
        } else {
            if (overloads.contains(overload_argcnt)) {
                throw std::runtime_error("Ambiguous overloads: Multiple overloads with the same number of arguments (" +
                                         std::to_string(overload_argcnt) + ") are not allowed for function " + name);
            }
            if (variadic_overload.has_value() && overload_argcnt >= variadic_overload.value().m_Args.size()) {
                throw std::runtime_error(
                        ambiguousOverload(name, variadic_overload.value().m_Args.size() - 1, overload_argcnt));
            }
            overloads.emplace(overload_argcnt, std::move(overload));
        }
    }
    if (overloads.empty() && !variadic_overload.has_value()) {
        throw std::runtime_error("A function must have at least one overload");
    }

    auto captures = std::move(ctx.m_CapturesMappingStack.back());
    ctx.m_CapturesMappingStack.pop_back();
    auto referenced_global_names = std::move(ctx.m_ReferencedGlobalNamesStack.back());
    ctx.m_ReferencedGlobalNamesStack.pop_back();
    auto module_imports = std::move(ctx.m_ModuleImportsStack.back());
    ctx.m_ModuleImportsStack.pop_back();

    /*
    // register a reference to this function in the parent scope
    if (!ctx.m_ReferencedGlobalNamesStack.empty()) {
        ctx.m_ReferencedGlobalNamesStack.back().insert(name);
    }
    */
    // register import of this module in the parent scope
    if (!ctx.m_ModuleImportsStack.empty()) {
        ctx.m_ModuleImportsStack.back().insert(name);
    }

    auto fn_expr = std::make_unique<FunctionExpr>(
            name,
            std::move(overloads),
            std::move(variadic_overload),
            std::move(captures),
            std::move(referenced_global_names),
            std::move(module_imports));

    fn_expr->compile();

    return fn_expr;
}

/** This compiles the internal function that takes unpacked positional arguments (and closure environment in
 * case of a closure) and creates a stub function that has a consistent signature and can be called at runtime:
 * stub_fn(Object *self, size_t argcnt, Object **argv)
 */
std::string FunctionExpr::compile() const {
    using namespace llvm;
    CodegenContext ctx(m_Name);
    FunctionModule function_module(m_Name, m_ModuleImports);

    std::unordered_map<std::string, GlobalVariable *> global_vars = ModuleUtil::declareReferencedGlobals
            (ctx, m_ReferencedGlobalNames);

    ctx.m_GlobalVariableMap = global_vars;

    if (Runtime::getInstance().m_CompilingAOT) {
        function_module.emitImportsFile();
    }

    auto arglist_type = ctx.pointerArrayType(); // array of Object *
    auto argcnt_type = Type::getInt64Ty(*ctx.m_LLVMContext);
    auto return_type = ctx.pointerType();
    // the stub type is Object *stub(Object *self, size_t llvm_argcnt, Object **argv)
    FunctionType *stub_type = llvm::FunctionType::get(return_type, {ctx.pointerType(),
                                                                    argcnt_type,
                                                                    arglist_type}, false);
    Function *stub_fn = llvm::Function::Create(stub_type,
                                               llvm::Function::ExternalLinkage,
                                               m_StubName,
                                               *ctx.m_Module);

    ctx.m_CurrentFunctionStack.emplace_back(stub_fn);

    Argument *self_arg = stub_fn->getArg(0);
    self_arg->setName("self");
    Argument *argc_arg = stub_fn->getArg(1);
    argc_arg->setName("argc");
    Argument *argv_arg = stub_fn->getArg(2);
    argv_arg->setName("argv");

    // compile overloads and the stub function that dispatches to the correct overload based on the argument count
    std::unordered_map<size_t, Function *> internal_fns;
    for (const auto &[argcnt, overload]: m_Overloads) {
        Function *internal_fn = overload.compile(m_Name, ctx, m_Captures);
        internal_fns.emplace(argcnt, internal_fn);
    }
    Function *variadic_internal_fn = nullptr;
    if (m_VariadicOverload.has_value()) {
        variadic_internal_fn = m_VariadicOverload->compile(m_Name, ctx, m_Captures);
    }

    // create a switch that dispatches to the correct internal function based on the argument count
    BasicBlock *entry_block = ctx.createBasicBlock("entry");
    BasicBlock *default_block = ctx.createBasicBlock("argcnt_default");
    BasicBlock *wrong_argcnt_block = ctx.createBasicBlock("argcnt_wrong");

    ctx.m_IRBuilder.SetInsertPoint(entry_block);
    auto *sw = ctx.m_IRBuilder.CreateSwitch(argc_arg, default_block, internal_fns.size());
    // first, compare with the non-variadic overloads
    for (const auto &[argcnt, internal_fn]: internal_fns) {
        BasicBlock *case_block = ctx.createBasicBlock("argcnt_" + std::to_string(argcnt));
        sw->addCase(ConstantInt::get(argcnt_type, argcnt, false), case_block);
        ctx.m_IRBuilder.SetInsertPoint(case_block);
        std::vector<Value *> direct_args;
        for (size_t i = 0; i < argcnt; i++) {
            Value *slot = ctx.m_IRBuilder.CreateGEP(ctx.pointerType(),
                                                    stub_fn->getArg(2),
                                                    ctx.m_IRBuilder.getInt64(i));
            Value *arg = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), slot, "arg_" + std::to_string(i));
            direct_args.push_back(arg);
        }
        if (m_Overloads.at(argcnt).m_UsesClosureEnv) {
            Value *closure_env = TCClosure::emitGetEnv(ctx, self_arg);
            direct_args.push_back(closure_env);
        }
        llvm::Value *result = ctx.m_IRBuilder.CreateCall(internal_fn, direct_args, "overload_result");
        ctx.m_IRBuilder.CreateRet(result);
    }
    // if there is a variadic overload, compare with the minimum argument count for the variadic overload
    if (variadic_internal_fn) {
        // in the argcnt_default case
        ctx.m_IRBuilder.SetInsertPoint(default_block);
        BasicBlock *variadic_block = ctx.createBasicBlock("variadic_case");
        size_t variadic_fixed_argcnt = m_VariadicOverload->m_Args.size() - 1;
        Value *is_enough_args = ctx.m_IRBuilder.CreateICmpUGE(argc_arg,
                                                              ctx.m_IRBuilder.getInt64(variadic_fixed_argcnt),
                                                              "is_enough_args_for_variadic_case");
        ctx.m_IRBuilder.CreateCondBr(is_enough_args, variadic_block, wrong_argcnt_block);
        ctx.m_IRBuilder.SetInsertPoint(variadic_block);
        std::vector<Value *> direct_args;
        for (size_t i = 0; i < variadic_fixed_argcnt; i++) {
            Value *slot = ctx.m_IRBuilder.CreateGEP(ctx.pointerType(),
                                                    stub_fn->getArg(2),
                                                    ctx.m_IRBuilder.getInt64(i));
            Value *arg = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), slot, "arg_" + std::to_string(i));
            direct_args.push_back(arg);
        }
        AllocaInst *varargs_alloca = ctx.m_IRBuilder.CreateAlloca(ctx.pointerType(), nullptr, "varargs_alloca");
        // for variadic functions, the last argument is a Clojure list of the extra arguments
        // however, if there are no extra args, the vararg is nil
        BasicBlock *build_varargs_array_block = ctx.createBasicBlock("build_varargs_array");
        BasicBlock *varargs_empty_block = ctx.createBasicBlock("varargs_empty");
        BasicBlock *merge_block = ctx.createBasicBlock("varargs_merge");
        Value *extra_argcnt = ctx.m_IRBuilder.CreateSub(argc_arg,
                                                        ctx.m_IRBuilder.getInt64(variadic_fixed_argcnt),
                                                        "extra_argcnt");
        Value *has_extra_args = ctx.m_IRBuilder.CreateICmpUGT(extra_argcnt,
                                                              ctx.m_IRBuilder.getInt64(0),
                                                              "has_extra_args");
        ctx.m_IRBuilder.CreateCondBr(has_extra_args, build_varargs_array_block, varargs_empty_block);

        // no extra varargs -> the packed list is nil
        ctx.m_IRBuilder.SetInsertPoint(varargs_empty_block);
        Value *nil_val = ConstantPointerNull::get(ctx.pointerType());
        ctx.m_IRBuilder.CreateStore(nil_val, varargs_alloca);
        ctx.m_IRBuilder.CreateBr(merge_block);

        // otherwise, build the varargs list from the extra arguments in the packed array
        ctx.m_IRBuilder.SetInsertPoint(build_varargs_array_block);
        Value *varargs_array_ptr = ctx.m_IRBuilder.CreateGEP(ctx.pointerType(),
                                                             argv_arg,
                                                             ctx.m_IRBuilder.getInt64(variadic_fixed_argcnt),
                                                             "extra_arglist_ptr");
        FunctionType *list_from_array_fn_type = FunctionType::get(ctx.pointerType(),
                                                                  {argcnt_type, arglist_type}, false);
        FunctionCallee list_from_array_fn = ctx.m_Module->getOrInsertFunction(
                "tc_list_from_array", list_from_array_fn_type);
        Value *varargs_list = ctx.m_IRBuilder.CreateCall(
                list_from_array_fn,
                {extra_argcnt, varargs_array_ptr},
                "varargs_list");
        ctx.m_IRBuilder.CreateStore(varargs_list, varargs_alloca);
        ctx.m_IRBuilder.CreateBr(merge_block);

        ctx.m_IRBuilder.SetInsertPoint(merge_block);
        Value *varargs_val = ctx.m_IRBuilder.CreateLoad(ctx.pointerType(), varargs_alloca, "varargs_val");
        direct_args.push_back(varargs_val);
        if (m_VariadicOverload->m_UsesClosureEnv) {
            Value *closure_env = TCClosure::emitGetEnv(ctx, self_arg);
            direct_args.push_back(closure_env);
        }
        llvm::Value *result = ctx.m_IRBuilder.CreateCall(variadic_internal_fn, direct_args, "variadic_overload_result");
        ctx.m_IRBuilder.CreateRet(result);
    } else {
        // no variadic case -> the default case is wrong argument count
        ctx.m_IRBuilder.SetInsertPoint(default_block);
        ctx.m_IRBuilder.CreateBr(wrong_argcnt_block);
    }

    // wrong argument count -> print error and exit
    ctx.m_IRBuilder.SetInsertPoint(wrong_argcnt_block);
    // todo: format the error message with the function name and the wrong argument count
    // const char *wrong_argcnt_fmt = "Error: Wrong number of arguments (%u) passed to a function %s\n";
    const char *wrong_argcnt_fmt = "Error: Wrong number of arguments passed to a function\n";
    Value *wrong_argcnt_msg = ctx.registerGlobalString(wrong_argcnt_fmt);
    //Value *fn_name_msg = ctx.registerGlobalString(m_Name);
    CompilerUtils::emitThrow(wrong_argcnt_msg, ctx);

    Function *module_init_fn = ModuleUtil::initReferencedGlobals(ctx, m_Name, global_vars);
    function_module.createModuleVtable(ctx, stub_fn);

    if (Runtime::getInstance().m_CompilingAOT) {
        function_module.writeModule(ctx);
    }

    ctx.linkModule(m_Name);

    // load module by calling the load function
    auto &rt = Runtime::getInstance();
    auto &jit = rt.getJIT();
    auto &aot_engine = rt.getAotEngine();
    using LoadFnType = void (*)();
    auto init_fn = reinterpret_cast<LoadFnType>(jit->lookup(util::module_init_fn_name(m_Name))->getAddress());

    aot_engine.startLoading(m_Name);
    init_fn();
    aot_engine.finishLoading(m_Name);

    ctx.m_CurrentFunctionStack.pop_back();
    return m_StubName;
}

EmitResult FunctionExpr::emitIR(CodegenContext &ctx) const {
    using namespace llvm;
    // For simple functions (without captures), the compiler needs to emit instructions to create a function object
    // that points to the stub and store it into dst

    // For closures, the compiler needs to emit instructions to create an environment struct from the captured
    // variables, then create a closure object that points to the stub and store it into dst
    FunctionType *stub_fn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), Type::getInt64Ty(*ctx.m_LLVMContext), ctx.pointerArrayType()},
            false);
    llvm::Function *stub_fn = llvm::Function::Create(stub_fn_type,
                                                     llvm::Function::ExternalLinkage,
                                                     m_StubName,
                                                     *ctx.m_Module);

    // Object *stub_fn(Object *self, size_t arg, Object **argv)
    FunctionType *call_fn_type = FunctionType::get(
            ctx.pointerType(),
            {ctx.pointerType(), Type::getInt64Ty(*ctx.m_LLVMContext), ctx.pointerArrayType()},
            false);

    // load vtable from m_VtableName global variable
    llvm::GlobalVariable *vtable = ctx.m_Module->getGlobalVariable(m_VtableName);
    if (!vtable) {
        // need to declare the vtable which is defined in the function's module (different from the current module)
        llvm::StructType *vtable_struct_type = Object::getMethodTableStructType(*ctx.m_LLVMContext);
        vtable = new llvm::GlobalVariable(
                *ctx.m_Module,
                vtable_struct_type,
                true, // isConstant
                llvm::GlobalValue::ExternalLinkage,
                nullptr, // initializer will be provided by the function's module
                m_VtableName);
    }

    if (isClosure()) {
        // allocate the closure environment struct on the heap (the env struct is an array of Object *)
        FunctionType *allocate_env_fn_type = FunctionType::get(
                ctx.pointerType(),
                {Type::getInt64Ty(*ctx.m_LLVMContext)}, false);
        FunctionCallee allocate_env_fn = ctx.m_Module->getOrInsertFunction("tc_closure_allocate_env",
                                                                           allocate_env_fn_type);
        FunctionType *closure_new_fn_type = FunctionType::get(
                ctx.pointerType(),
                // CallFn callStub, Object **env, size_t numCaptures
                {PointerType::get(call_fn_type, 0), ctx.pointerType(), Type::getInt64Ty(*ctx.m_LLVMContext)},
                false);
        FunctionCallee closure_new_fn = ctx.m_Module->getOrInsertFunction("tc_closure_new", closure_new_fn_type);
        Value *num_captures_val = ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext), m_Captures.size(), false);
        Value *env_struct_ptr = ctx.m_IRBuilder.CreateCall(
                allocate_env_fn,
                {ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext), m_Captures.size(), false)},
                "closure_env");
        // store captured variables into the env struct
        for (const auto &[var_name, captured_local_expr]: m_Captures) {
            auto &origin = captured_local_expr.getOrigin();
            EmitResult captured_var_value = origin->emitIR(ctx);
            Value *env_slot_ptr = ctx.m_IRBuilder.CreateGEP(
                    ctx.pointerType(),
                    env_struct_ptr,
                    ConstantInt::get(Type::getInt64Ty(*ctx.m_LLVMContext),
                                     captured_local_expr.getClosureEnvIndex(),
                                     false),
                    var_name + "_env_slot_ptr");
            ctx.m_IRBuilder.CreateStore(captured_var_value.value(), env_slot_ptr);
        }

        // create the closure object with the vtable pointer and env struct pointer
        return ctx.m_IRBuilder.CreateCall(closure_new_fn,
                                          {vtable, env_struct_ptr, num_captures_val},
                                          "closure_obj");
    } else {
        // create the function object with the stub pointer
        FunctionType *function_new_fn_type = FunctionType::get(
                ctx.pointerType(),
                {PointerType::get(call_fn_type, 0), Type::getInt8PtrTy(*ctx.m_LLVMContext)},
                false);
        FunctionCallee function_new_fn = ctx.m_Module->getOrInsertFunction("tc_function_new", function_new_fn_type);
        Value *fn_name_const = ctx.registerGlobalString(m_Name);
        return ctx.m_IRBuilder.CreateCall(function_new_fn, {vtable, fn_name_const}, "function_obj");
    }
}

bool FunctionExpr::isClosure() const {
    return !m_Captures.empty();
}

const Object *FunctionExpr::eval() const {
    auto &jit = Runtime::getInstance().getJIT();

/*
    const CallFn stub_fn = reinterpret_cast<const CallFn>(
            jit->lookup(m_StubName)->getAddress());
*/
    const MethodTable *vtable = reinterpret_cast<const MethodTable *>(jit->lookup(m_VtableName)->getAddress());
    return tc_function_new(vtable, m_Name.c_str());
}
