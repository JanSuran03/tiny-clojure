find_program(BB_EXE bb REQUIRED)

add_executable(test_reader
        test_reader.cpp
)

target_link_libraries(test_reader
        PRIVATE tinyclj_lib
)

add_test(NAME test_reader
        COMMAND test_reader
)

add_test(NAME test_integration
        COMMAND ${BB_EXE} ${PROJECT_SOURCE_DIR}/test/test.bb
)

add_test(NAME run_benchmarks
        COMMAND ${BB_EXE} ${PROJECT_SOURCE_DIR}/bench/run-benchmark.bb
)
