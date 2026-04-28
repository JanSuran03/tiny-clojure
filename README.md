# TinyClojure

Native implementation of a very small subset of Clojure on top of LLVM.

## Building TinyClojure

### Dependencies:

- [CMake](https://cmake.org/download/), Minimum version 3.25.1
- LLVM: The project is tested with LLVM [14.0.6](https://github.com/llvm/llvm-project/releases/tag/llvmorg-14.0.6).
  LLVM can break backwards compatibility so it's recommended to use the tested version, newer versions may work but are
  not guaranteed to be compatible.
- A C++20 compatible compiler
  [main.cpp](main.cpp)

### Build Instructions:

```shell
cmake -S . -B build # Configure the project in the current directory and generate build files in the 'build' directory
cmake --build build # Build the project using the generated build files in the 'build' directory
```

### Running the REPL:

```shell
./build/tiny-clojure <command-line-arguments>
```

### Additional command-line arguments:

- `--help`: Shows help message with available command-line arguments and exits.
- `--suppress-repl-welcome`: Suppresses the REPL welcome message when starting the REPL,
  making it more suitable for automated testing for example.

## Testing:

### Additional dependencies:

- [Babashka](https://github.com/babashka/babashka#installation) - a fast and lightweight Clojure interpreter for
  testing,
  Minimal features are used, so any non-legacy version should work.

### Run tests:

```shell
ctest --test-dir build --output-on-failure # Run tests and output results on failure
```
