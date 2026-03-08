#include <iostream>

#include "compiler/ast/parser.h"
#include "runtime/rt.h"
#include "types/TCList.h"
#include "types/TCSymbol.h"
#include "types/TCInteger.h"

int main() {
    llvm::LLVMContext llvm_ctx;
    llvm::IRBuilder<> builder(llvm_ctx);
    llvm::Module module("hello_world_module", llvm_ctx);
    CompilerContext ctx(llvm_ctx, builder, module);

    // (let (a 1) a)
    const Object *bindings = tc_list_cons(tc_symbol_new("a"),
                                          tc_list_cons(tc_integer_new(1), empty_list()));
    const Object *form = tc_list_cons(tc_symbol_new("let"),
                                      tc_list_cons(bindings,
                                                   tc_list_cons(tc_symbol_new("a"),
                                                                empty_list())));
    AExpr expr = Parser::analyze(ctx, form);
    Object *res = expr->eval();
    tinyclj_rt_print(res);

    //std::cout << "Hello, World!" << std::endl;

    return 0;
}
