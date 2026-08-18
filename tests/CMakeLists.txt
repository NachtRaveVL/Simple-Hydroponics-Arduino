cmake_minimum_required(VERSION 3.16)
project(HydruinoHostTests LANGUAGES CXX)

enable_testing()

add_executable(hydruino_core_tests
    host/test_core_logic.cpp
)
target_include_directories(hydruino_core_tests PRIVATE ../src)
target_compile_features(hydruino_core_tests PRIVATE cxx_std_17)

add_test(NAME hydruino_core_tests COMMAND hydruino_core_tests)
