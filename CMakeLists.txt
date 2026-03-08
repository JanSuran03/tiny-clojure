cmake_minimum_required(VERSION 3.25.1)
project(tiny-clojure)

set(CMAKE_CXX_STANDARD 23)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(tiny-clojure main.cpp
        tinyclj/compiler/CompilerContext.cpp
        tinyclj/compiler/ast/BodyExpr.cpp
        tinyclj/compiler/ast/BooleanExpr.cpp
        tinyclj/compiler/ast/CharExpr.cpp
        tinyclj/compiler/ast/ConstantExpr.cpp
        tinyclj/compiler/ast/DefExpr.cpp
        tinyclj/compiler/ast/DoubleExpr.cpp
        tinyclj/compiler/ast/FunctionExpr.cpp
        tinyclj/compiler/ast/IfExpr.cpp
        tinyclj/compiler/ast/IntegerExpr.cpp
        tinyclj/compiler/ast/LetExpr.cpp
        tinyclj/compiler/ast/NilExpr.cpp
        tinyclj/compiler/ast/QuotedExpr.cpp
        tinyclj/compiler/ast/StringExpr.cpp
        tinyclj/compiler/ast/UnevaluatableExpr.cpp
        tinyclj/compiler/ast/VarExpr.cpp
        tinyclj/compiler/ast/parser.cpp
        tinyclj/runtime/rt.cpp
        tinyclj/types/Object.cpp
        tinyclj/types/TCBoolean.cpp
        tinyclj/types/TCChar.cpp
        tinyclj/types/TCDouble.cpp
        tinyclj/types/TCInteger.cpp
        tinyclj/types/TCList.cpp
        tinyclj/types/TCString.cpp
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
