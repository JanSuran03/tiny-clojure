cmake_minimum_required(VERSION 3.25.1)
project(tiny-clojure)

set(CMAKE_CXX_STANDARD 23)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(tiny-clojure main.cpp
        tinyclj/types/Object.cpp
        tinyclj/types/TCDouble.cpp
        tinyclj/types/TCInteger.cpp
        tinyclj/types/TCList.cpp
        tinyclj/types/TCSymbol.cpp
)

llvm_map_components_to_libnames(LLVM_LIBS
        core
        support
        irreader

        orcjit
        native
        nativecodegen

        executionengine
        runtimedyld
        mc
        object
        passes
        analysis
        scalaropts
        transformutils
        bitreader
)

target_link_libraries(tiny-clojure ${LLVM_LIBS})
