add_executable(test-runtime test-runtime.cpp)

target_link_libraries(test-runtime PRIVATE tinyclj_lib)

target_compile_definitions(test-runtime PRIVATE
        PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_test(NAME runtime-test COMMAND test-runtime)

find_program(BB_EXE bb REQUIRED)

add_test(NAME run_hello
        COMMAND ${BB_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test-repl.bb
)
