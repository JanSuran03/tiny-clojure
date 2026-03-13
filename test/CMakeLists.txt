add_executable(test-runtime test-runtime.cpp)

target_link_libraries(test-runtime PRIVATE tinyclj_lib)

target_compile_definitions(test-runtime PRIVATE
        PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_test(NAME runtime-test COMMAND test-runtime)
