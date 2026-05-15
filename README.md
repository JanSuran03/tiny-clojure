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
```[main.cpp](main.cpp)
```

### Additional command-line arguments:

- `--help`: Shows help message with available command-line arguments and exits.
- `--suppress-repl-welcome`: Suppresses the REPL welcome message when starting the REPL,
   making it more suitable for automated testing for example.
- `--disable-gc`: Disables garbage collection, which can be useful for debugging or testing purposes.
- `-opt-level <level>`: Sets the optimization level for code generation.
   Valid levels are `O0`, `O1`, `O2` and `O3`. The default level is `O2`.
- `-compiled-dir <directory>`: Specifies the directory where compiled code will be stored. 
   The default directory is `compiled`.
- `--direct-linking`: Enables direct linking of compiled code, which can improve performance
   by avoiding the overhead of dynamic linking. This option is always enabled when compiling
   the standard library; otherwise it is disabled by default and experimental.
- `-int-cache-range <from:to>`: Specifies the range of integers to cache for optimization purposes.
The default range is `-128:127`, the limits are `-65536:65535`. Caching integers can improve
   performance by avoiding the overhead of creating new integer objects for frequently used values.
- `-user-code-dir <directory>`: Specifies the directory where additional user code is located.
   The module loader first searches in the source directory for the standard library, then in the
   user code directory. This allows users to easily add their own code without modifying the standard
   library.

## Testing:

### Additional dependencies:

- [Babashka](https://github.com/babashka/babashka#installation) - a fast and lightweight Clojure interpreter for
  testing,
  Minimal features are used, so any non-legacy version should work.

### Run tests:

```shell
ctest --test-dir build --output-on-failure # Run tests and output results on failure
```

### Benchmarking
Benchmarking is performed together with testing. Benchmark results are stored in the
`bench/dump` directory.

After these dump files are generated, inside the `bench` directory, you can run

```shell
python3 visualize.py
```
This will generate graphs from the benchmark results and save them in the `bench/figures` directory.

### License
Distributed under the Eclipse Public License (EPL-2.0), the same one as Clojure.

### Affiliation
<img src="https://fit.cvut.cz/static/images/fit-cvut-logo-en.svg" alt="FIT CTU logo" height="200">

This software was developed with the support of the **Faculty of Information Technology, Czech Technical University in Prague**.
For more information, visit [fit.cvut.cz](https://fit.cvut.cz).
