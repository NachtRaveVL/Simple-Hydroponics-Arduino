cmake_minimum_required(VERSION 3.16)
project(HydruinoHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(Python3 COMPONENTS Interpreter)

enable_testing()

add_executable(hydruino_core_tests
    host/test_core_logic.cpp
)
target_include_directories(hydruino_core_tests PRIVATE ../src)

add_test(NAME hydruino_core_tests COMMAND hydruino_core_tests)

if(Python3_Interpreter_FOUND)
    add_test(NAME source_validation COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/validate_source.py)
endif()
