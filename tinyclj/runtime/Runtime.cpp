#include <fstream>
#include <iostream>
#include <sstream>

#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"

#include "Runtime.h"
#include "compiler/ast/DefExpr.h"
#include "compiler/SemanticAnalyzer.h"
#include "reader/LispReader.h"
#include "runtime/rt.h"
#include "types/TCFunction.h"
#include "types/TCSymbol.h"

void Runtime::ensureInitialized() {
    if (m_Initialized) return;

    if (m_InitInProgress) {
        // maybe allow this for now?
        return;
    }

    m_InitInProgress = true;
    init();
    m_InitInProgress = false;
    m_Initialized = true;
}

std::unique_ptr<llvm::orc::LLJIT> Runtime::createJIT() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto create_result = llvm::orc::LLJITBuilder().create();
    if (!create_result) {
        throw std::runtime_error("Failed to create JIT: " + llvm::toString(create_result.takeError()));
    }

    auto jit = std::move(*create_result);

    auto &jd = jit->getMainJITDylib();
    auto dl = jit->getDataLayout();

    auto gen_result = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix());

    if (!gen_result) {
        throw std::runtime_error("Failed to create DynamicLibrarySearchGenerator: "
                                 + llvm::toString(gen_result.takeError()));
    }

    jd.addGenerator(std::move(*gen_result));
    return jit;
}

// todo: are both methods needed?
const std::unordered_map<std::string, Object *> &Runtime::getGlobalVarStorage() const {
    return m_GlobalVarStorage;
}

std::unordered_map<std::string, Object *> &Runtime::getGlobalVarStorage() {
    return m_GlobalVarStorage;
}

Object *Runtime::declareVar(const std::string &name) {
    if (auto it = m_GlobalVarStorage.find(name); it != m_GlobalVarStorage.end()) {
        return it->second;
    } else {
        return m_GlobalVarStorage[name] = tc_var_new(name.c_str());
    }
}

Object *Runtime::getVar(const std::string &name) const {
    if (auto var = m_GlobalVarStorage.find(name); var != m_GlobalVarStorage.end()) {
        return var->second;
    } else {
        return nullptr;
    }
}

void Runtime::defn(const std::string &name, CallFn fn) {
    auto var = declareVar(name);
    tc_var_bind_root(var, tc_function_new(fn, name.c_str()));
}

Object *Runtime::createObject(ObjectType type, void *data, CallFn callFn, bool isStatic) {
    return m_Heap.createObject(type, data, callFn, isStatic);
}

void Runtime::init() {
    defn("builtin_binary_add", tinyclj_rt_add);
    defn("builtin_binary_sub", tinyclj_rt_sub);
    defn("builtin_binary_mul", tinyclj_rt_mul);
    defn("builtin_binary_div", tinyclj_rt_div);
    defn("builtin_unary_print", tinyclj_rt_print);
    defn("builtin_binary_equal", tinyclj_rt_binary_equal);
    defn("builtin_binary_lt", tinyclj_rt_lt);
    defn("builtin_binary_lte", tinyclj_rt_lte);
    defn("set-macro!", tinyclj_rt_setmacro);
    defn("identical?", tinyclj_rt_identical);
    defn("list", tinyclj_rt_list);
    defn("cons", tinyclj_rt_cons);
    defn("next", tinyclj_rt_next);
    defn("seq", tinyclj_rt_seq);
    defn("list*", tinyclj_rt_list_STAR);
    defn("count", tinyclj_rt_count);
    defn("first", tinyclj_rt_first);
    defn("error", tinyclj_rt_error);
    defn("nil?", tinyclj_rt_is_nil);
    defn("string?", tinyclj_rt_is_string);
    defn("symbol?", tinyclj_rt_is_symbol);
    defn("list?", tinyclj_rt_is_list);
    defn("function?", tinyclj_rt_is_function);
    defn("integer?", tinyclj_rt_is_integer);
    defn("double?", tinyclj_rt_is_double);
    defn("boolean?", tinyclj_rt_is_boolean);
    defn("var?", tinyclj_rt_is_var);
    defn("character?", tinyclj_rt_is_character);
    defn("apply", tinyclj_rt_apply);
    defn("macroexpand", tinyclj_rt_macroexpand);
    defn("macroexpand1", tinyclj_rt_macroexpand1);
    defn("eval", tinyclj_rt_eval);
    defn("vars", tinyclj_rt_vars);
    defn("epoch-nanos", tinyclj_rt_epoch_nanos);
    defn("next-id", tinyclj_rt_nextID);
    defn("symbol", tinyclj_rt_symbol);
    defn("str", tinyclj_rt_str);
    defn("double", tinyclj_rt_double);
    defn("long", tinyclj_rt_long);

    static const std::string core_file = PROJECT_SOURCE_DIR + std::string("/core.clj");
    try {
        loadFile(core_file);
    } catch (const std::runtime_error &e) {
        std::cerr << "Warning: Failed to load core.clj: " << e.what() << std::endl;
        throw;
    }
}

Runtime::Runtime()
        : m_JIT(createJIT()) {}

Runtime &Runtime::getInstance() {
    static Runtime instance;
    instance.ensureInitialized();
    return instance;
}

size_t Runtime::nextId() {
    return m_IdCounter++;
}

std::unique_ptr<llvm::orc::LLJIT> &Runtime::getJIT() {
    return m_JIT;
}

Object *Runtime::eval(const Object *form) {
    std::unique_ptr<llvm::LLVMContext> llvm_ctx = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("eval_module", *llvm_ctx);
    llvm::IRBuilder<> builder(*llvm_ctx);

    CompilerContext ctx(*this, *llvm_ctx, builder, *module, m_IdCounter);

    // wrap the code in an anonymous function call (for now, for all forms), then evaluate that function
    // (-> (fn* fn_name [] form))

    // todo: macroexpand and do some branching

    //if (form && form->m_Type == ObjectType::sym_list) {
    //    const Object *op = tc_list_first(form);
    //    if (op != nullptr) {
    //        const char *name = static_cast<TCSymbol *>(op->m_Data)->m_Name;
    //        if (!strcmp(name, "do")) {
    //            Object *res = nullptr;
    //            for (const Object *forms = tc_list_next(form); forms; forms = tc_list_next(forms)) {
    //                res = eval(tc_list_first(forms));
    //            }
    //            return res;
    //        } else if (!strcmp(name, "def")) {
    //            throw std::runtime_error("cannot eval def (yet)");
    //        }
    //    }
    //}
    const Object *new_form = tc_list_create3(tc_symbol_new("fn*"), empty_list(), form);

    AExpr expr = SemanticAnalyzer::analyze(ctx, new_form);

    if (llvm::verifyModule(ctx.m_Module, &llvm::errs())) {
        ctx.m_Module.dump();
        throw std::runtime_error("Module verification failed");
    }

    llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(llvm_ctx));

    if (auto err = m_JIT->addIRModule(std::move(tsm))) {
        throw std::runtime_error("Failed to add module to JIT: " + llvm::toString(std::move(err)));
    }

    const Object *evaled_wrapper_fn = expr->eval(*this);
    return evaled_wrapper_fn->m_Call(evaled_wrapper_fn, 0, nullptr);
}

void Runtime::registerConstant(const Object *obj) {
    m_ConstantObjects.emplace(obj);
}

const std::unordered_set<const Object *> &Runtime::getConstantObjects() const {
    return m_ConstantObjects;
}

void Runtime::repl() {
    BufferedReader reader(std::cin);

    std::cout << "Welcome to TinyCLJ REPL!" << std::endl
              << "Type exit or Ctrl+D to quit." << std::endl
              << "Type (vars) to see all defined vars." << std::endl;

    while (true) {
        std::cout << "> " << std::flush;
        const Object *form = LispReader::read(reader);
        if (form == LispReader::eof_object()
            || (form != nullptr
                && form->m_Type == ObjectType::SYMBOL
                && strcmp(static_cast<const TCSymbol *>(form->m_Data)->m_Name, "exit") == 0)) {
            std::cout << "Goodbye!" << std::endl;
            break;
        }

        try {
            const Object *res = eval(form);
            std::cout << "=> ";
            tinyclj_rt_print(res, 1, &res);
            std::cout << std::endl;
            m_Heap.collectGarbageIfNeeded(this);
        } catch (const std::runtime_error &e) {
            std::cout << "Runtime error: " << e.what() << std::endl;
        }
    }
}

const Object *Runtime::loadStream(std::istream &stream) {
    BufferedReader reader(stream);
    const Object *res = nullptr;
    while (true) {
        const Object *form = LispReader::read(reader);
        if (form == LispReader::eof_object()) {
            break;
        }
        res = eval(form);
        {
            RootFrameGuard guard(*this, {const_cast<Object *>(res)});
            m_Heap.collectGarbageIfNeeded(this);
        }
    }
    return res;
}

const Object *Runtime::readString(const std::string &input) {
    std::istringstream stream(input);
    BufferedReader reader(stream);
    return LispReader::read(reader);
}

const Object *Runtime::loadString(const std::string &input) {
    std::istringstream stream(input);
    return loadStream(stream);
}

const Object *Runtime::loadFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    return loadStream(file);
}
