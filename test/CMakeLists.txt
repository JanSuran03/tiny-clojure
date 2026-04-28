find_program(BB_EXE bb REQUIRED)

add_test(NAME run_tests
        COMMAND ${BB_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test.bb
)

add_test(NAME run_benchmarks
        COMMAND ${BB_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/run-benchmark.bb
)
