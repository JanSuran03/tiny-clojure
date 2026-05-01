#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include "llvm/Support/TargetSelect.h"

#include "Runtime.h"
#include "compiler/ast/local-binding/CapturedLocalExpr.h"
#include "compiler/AnalyzerContext.h"
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

template<CallFn Fn>
void Runtime::defn(const std::string &name, const char *nativeFnName) {
    getAotEngine().startLoading(name);
    auto var = declareVar(name);
    tc_var_bind_root(var, tc_function_new(&BuiltinFunctionVTable<Fn>::value, nativeFnName));
    getAotEngine().finishLoading(name);
}

#define TC_DEFN(name, nativeFn) defn<nativeFn>(name, #nativeFn)

std::filesystem::file_time_type Runtime::computeLastSourceWriteTime() {
    static std::string source_root = std::string(TINYCLJ_PROJECT_SOURCE_DIR) + "/tinyclj";
    std::filesystem::file_time_type last_write_time = std::filesystem::file_time_type::min();
    for (const auto &entry: std::filesystem::recursive_directory_iterator(source_root)) {
        if (entry.is_regular_file()) {
            auto entry_write_time = entry.last_write_time();
            if (entry_write_time > last_write_time) {
                last_write_time = entry_write_time;
            }
        }
    }
    return last_write_time;
}

std::filesystem::file_time_type Runtime::getSourceLastWriteTime() const {
    return m_SourceLastWriteTime;
}

Object *Runtime::createObject(ObjectType type, void *data, const MethodTable *methodTable, bool isStatic) {
    return m_Heap.createObject(type, data, methodTable, isStatic);
}

/// Creates the directory if it doesn't exist and clears all its contests.
void clear_directory(const std::filesystem::path &path) {
    namespace fs = std::filesystem;

    fs::create_directories(path);

    for (const auto &entry: fs::directory_iterator(path)) {
        fs::remove_all(entry.path());
    }
}

void Runtime::init() {
    TCBoolean::init();

    // Todo: clear the compiled directory if the source files have been updated since the last compilation.
    //clear_directory(std::filesystem::path(m_AotEngine.m_CompiledRoot));

    TC_DEFN("builtin_binary_add", tinyclj_rt_add);
    TC_DEFN("builtin_binary_sub", tinyclj_rt_sub);
    TC_DEFN("builtin_binary_mul", tinyclj_rt_mul);
    TC_DEFN("builtin_binary_div", tinyclj_rt_div);
    TC_DEFN("builtin_unary_print", tinyclj_rt_print);
    TC_DEFN("flush", tinyclj_rt_flush);
    TC_DEFN("to-edn", tinyclj_rt_to_edn);
    TC_DEFN("builtin_binary_equal", tinyclj_rt_binary_equal);
    TC_DEFN("builtin_binary_lt", tinyclj_rt_lt);
    TC_DEFN("builtin_binary_lte", tinyclj_rt_lte);
    TC_DEFN("set-macro!", tinyclj_rt_setmacro);
    TC_DEFN("identical?", tinyclj_rt_identical);
    TC_DEFN("list", tinyclj_rt_list);
    TC_DEFN("cons", tinyclj_rt_cons);
    TC_DEFN("next", tinyclj_rt_next);
    TC_DEFN("seq", tinyclj_rt_seq);
    TC_DEFN("list*", tinyclj_rt_list_STAR);
    TC_DEFN("count", tinyclj_rt_count);
    TC_DEFN("first", tinyclj_rt_first);
    TC_DEFN("error", tinyclj_rt_error);
    TC_DEFN("nil?", tinyclj_rt_is_nil);
    TC_DEFN("string?", tinyclj_rt_is_string);
    TC_DEFN("symbol?", tinyclj_rt_is_symbol);
    TC_DEFN("list?", tinyclj_rt_is_list);
    TC_DEFN("function?", tinyclj_rt_is_function);
    TC_DEFN("integer?", tinyclj_rt_is_integer);
    TC_DEFN("double?", tinyclj_rt_is_double);
    TC_DEFN("boolean?", tinyclj_rt_is_boolean);
    TC_DEFN("var?", tinyclj_rt_is_var);
    TC_DEFN("character?", tinyclj_rt_is_character);
    TC_DEFN("apply", tinyclj_rt_apply);
    TC_DEFN("read", tinyclj_rt_read);
    TC_DEFN("read-string", tinyclj_rt_read_string);
    TC_DEFN("slurp", tinyclj_rt_slurp);
    TC_DEFN("spit", tinyclj_rt_spit);
    TC_DEFN("macroexpand", tinyclj_rt_macroexpand);
    TC_DEFN("macroexpand1", tinyclj_rt_macroexpand1);
    TC_DEFN("eval", tinyclj_rt_eval);
    TC_DEFN("vars", tinyclj_rt_vars);
    TC_DEFN("epoch-nanos", tinyclj_rt_epoch_nanos);
    TC_DEFN("next-id", tinyclj_rt_nextID);
    TC_DEFN("symbol", tinyclj_rt_symbol);
    TC_DEFN("str", tinyclj_rt_str);
    TC_DEFN("double", tinyclj_rt_double);
    TC_DEFN("long", tinyclj_rt_long);
    TC_DEFN("compile-module", tinyclj_rt_compile_module);
    TC_DEFN("load-module", tinyclj_rt_load_module);

    static const std::string core_module = "core";
    try {
        AotEngine &aot_engine = getAotEngine();
        auto ts1 = std::chrono::high_resolution_clock::now();
        m_DirectLinking = true;
        aot_engine.compileModule(core_module, false);
        m_DirectLinking = false;
        auto ts2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = ts2 - ts1;
        // disable this log for now
        if (false) {
            std::cout << "Loaded core.clj in " << elapsed.count() << " seconds." << std::endl;
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Warning: Failed to load core.clj: " << e.what() << std::endl;
        throw;
    }
}

Runtime::Runtime()
        : m_JIT(createJIT()),
          m_SourceLastWriteTime(computeLastSourceWriteTime()) {}

Runtime &Runtime::getInstance() {
    static Runtime instance;
    instance.ensureInitialized();
    return instance;
}

size_t Runtime::nextId() {
    /*return m_IdCounter++;*/
    // for now, use epoch nanos to generate IDs
    auto ts = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ts).count();
}

std::unique_ptr<llvm::orc::LLJIT> &Runtime::getJIT() {
    return m_JIT;
}

const Object *Runtime::eval(const Object *form) {
    form = SemanticAnalyzer::macroexpand(Runtime::getInstance(), form);
    const Object *original_form = form;
    bool is_wrapped = false;
    // do -> eval sequentially
    // (f x y z) -> wrap in a fn (fn () (f x y z)), compile the function and call it
    // otherwise -> eval directly (e.g. literals)
    if (form && form->m_Type == ObjectType::LIST) {
        const Object *op = tc_list_first(form);
        if (op != nullptr && op->m_Type == ObjectType::SYMBOL) {
            const char *name = static_cast<TCSymbol *>(op->m_Data)->m_Name;
            if (strcmp(name, "do") == 0) {
                const Object *res = nullptr;
                for (const Object *forms = tc_list_next(form); forms; forms = tc_list_next(forms)) {
                    res = eval(tc_list_first(forms));
                }
                return res;
            } else if (strcmp(name, "def") == 0) {
                goto not_wrapped;
            }
        }
    }
    form = tc_list_create3(tc_symbol_new("__eval_fn_wrapper"), empty_list(), form);
    is_wrapped = true;

    not_wrapped:

    AnalyzerContext analyzer_ctx;
    AExpr expr = SemanticAnalyzer::analyze(analyzer_ctx, form);

    const Object *evaled_wrapper_fn = expr->eval();
    if (is_wrapped) {
        return evaled_wrapper_fn->m_MethodTable->m_CallFn(evaled_wrapper_fn, 0, nullptr);
    } else {
        return evaled_wrapper_fn;
    }
}

AotEngine &Runtime::getAotEngine() {
    return m_AotEngine;
}

void Runtime::repl() {
    Runtime &rt = Runtime::getInstance();

    if (!rt.m_SuppressReplWelcome) {
        std::cout << "Welcome to TinyCLJ REPL!" << std::endl
                  << "Type exit or Ctrl+D to quit." << std::endl
                  << "Type (vars) to see all defined vars." << std::endl;
    }
    while (true) {
        std::cout << "user=> " << std::flush;
        BufferedReader reader(std::cin);
        try {
            const Object *form = LispReader::read(reader);
            if (form == LispReader::eof_object()
                || (form != nullptr
                    && form->m_Type == ObjectType::SYMBOL
                    && strcmp(static_cast<const TCSymbol *>(form->m_Data)->m_Name, "exit") == 0)) {
                std::cout << std::endl;
                if (!rt.m_SuppressReplWelcome) {
                    std::cout << "Goodbye!" << std::endl;
                }
                break;
            }
            const Object *res = eval(form);
            const Object *as_edn = tinyclj_rt_to_edn(nullptr, 1, &res);
            tinyclj_rt_print(nullptr, 1, &as_edn);
            std::cout << std::endl;
            Runtime::getInstance().m_Heap.collectGarbageIfNeeded();
        } catch (const std::runtime_error &e) {
            std::cerr << "Could not evaluate form due to a runtime error:\n" << e.what() << std::endl;
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
            RootFrameGuard guard({res});
            Runtime::getInstance().m_Heap.collectGarbageIfNeeded();
        }
    }
    return res;
}

const Object *Runtime::read() {
    BufferedReader reader(std::cin);
    return LispReader::read(reader);
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

const Object *Runtime::slurp(const std::string &filename) {
    std::ifstream file(TINYCLJ_PROJECT_SOURCE_DIR + std::string("/") + filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return tc_string_new(buffer.str().c_str());
}

void Runtime::spit(const std::string &filename, const std::string &content) {
    std::ofstream file(TINYCLJ_PROJECT_SOURCE_DIR + std::string("/") + filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file << content;
}

extern "C" {
Object *tc_runtime_declare_var(const char *name) {
    return Runtime::getInstance().declareVar(name);
}

void tc_runtime_load_module(const char *moduleName) {
    Runtime::getInstance().getAotEngine().loadCompiledModule(moduleName);
}
}
