#include "Runtime.h"

int main() {
    Runtime &runtime = Runtime::getInstance();
    runtime.repl();

    return 0;
}
