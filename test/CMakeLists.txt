find_program(BB_EXE bb REQUIRED)

add_executable(test_parser
        test_parser.cpp
)

target_link_libraries(test_parser
        PRIVATE tinyclj_lib
)

add_test(NAME test_parser
        COMMAND test_parser
)

add_test(NAME test_integration
        COMMAND ${BB_EXE} ${PROJECT_SOURCE_DIR}/test/test.bb
)

add_test(NAME run_benchmarks
        COMMAND ${BB_EXE} ${PROJECT_SOURCE_DIR}/bench/run-benchmark.bb
)
