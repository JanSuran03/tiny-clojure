#include "Runtime.h"

#include "llvm/Support/TargetSelect.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    Runtime runtime;
    runtime.repl();

    return 0;
}
