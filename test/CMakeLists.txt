find_program(BB_EXE bb REQUIRED)

add_test(NAME run_tests
        COMMAND ${BB_EXE} test.bb
)

add_test(NAME run_benchmarks
        COMMAND ${BB_EXE} ${PROJECT_SOURCE_DIR}/bench/run-benchmark.bb
)
