#include <unordered_map>

#include "llvm/IR/Verifier.h"

#include "Runtime.h"
#include "compiler/ast/parser.h"
#include "runtime/rt.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"

std::unique_ptr<llvm::orc::LLJIT> Runtime::createJIT() {
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

Runtime::Runtime(const std::vector<std::string> &objectFiles)
        : m_JIT(createJIT()) {
    for (const auto &objectFile : objectFiles) {
        auto buffer_or_err = llvm::MemoryBuffer::getFile(objectFile);
        if (!buffer_or_err) {
            throw std::runtime_error("Failed to read object file: " + std::to_string(buffer_or_err.getError().value()));
        }

        if (auto err = m_JIT->addObjectFile(std::move(*buffer_or_err))) {
            throw std::runtime_error("Failed to add object file to JIT: " + llvm::toString(std::move(err)));
        }
    }
}

std::unique_ptr<llvm::orc::LLJIT> &Runtime::getJIT() {
    return m_JIT;
}

Object *Runtime::eval(const Object *form) {
    std::unique_ptr<llvm::LLVMContext> llvm_ctx = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("eval_module", *llvm_ctx);
    llvm::IRBuilder<> builder(*llvm_ctx);

    CompilerContext ctx(*llvm_ctx, builder, *module);

    // wrap the code in an anonymous function call (for now, for all forms), then evaluate that function
    // (-> (fn* fn_name [] form))
    const Object *new_form = tc_list_cons(tc_symbol_new("fn*"),
                                          tc_list_cons
                                                  (empty_list(),
                                                   tc_list_cons(form, empty_list())));

    AExpr expr = Parser::analyze(ctx, new_form);

    if (llvm::verifyModule(ctx.m_Module, &llvm::errs())) {
        ctx.m_Module.dump();
        throw std::runtime_error("Module verification failed");
    }

    llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(llvm_ctx));

    if (auto err = m_JIT->addIRModule(std::move(tsm))) {
        throw std::runtime_error("Failed to add module to JIT: " + llvm::toString(std::move(err)));
    }

    // print the llvm module
    return expr->eval(*this);
}
