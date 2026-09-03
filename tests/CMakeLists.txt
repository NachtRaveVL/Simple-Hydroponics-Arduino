cmake_minimum_required(VERSION 3.16)
project(HydruinoHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
endif()

find_package(Python3 COMPONENTS Interpreter)

set(HYDRUINO_SRC
    ../src/HydroActivation.cpp
    ../src/HydroActuators.cpp
    ../src/HydroAttachments.cpp
    ../src/HydroBalancers.cpp
    ../src/HydroCrops.cpp
    ../src/HydroCropsLibrary.cpp
    ../src/HydroData.cpp
    ../src/HydroDatas.cpp
    ../src/HydroFactory.cpp
    ../src/HydroLogger.cpp
    ../src/HydroMeasurements.cpp
    ../src/HydroModules.cpp
    ../src/HydroObject.cpp
    ../src/HydroPins.cpp
    ../src/HydroPublisher.cpp
    ../src/HydroRails.cpp
    ../src/HydroReservoirs.cpp
    ../src/HydroScheduler.cpp
    ../src/HydroSensors.cpp
    ../src/HydroStreams.cpp
    ../src/HydroStrings.cpp
    ../src/HydroTriggers.cpp
    ../src/HydroUtils.cpp
    ../src/Hydruino.cpp
    ../src/shared/HydroUIData.cpp
)

add_library(hydruino_host STATIC ${HYDRUINO_SRC})
target_include_directories(hydruino_host PUBLIC ../src)

add_executable(hydruino_core_tests
    host/test_core_logic.cpp
)
target_link_libraries(hydruino_core_tests PRIVATE hydruino_host)

add_executable(hydruino_hardening_tests
    host/test_hardening.cpp
)
target_link_libraries(hydruino_hardening_tests PRIVATE hydruino_host)

add_executable(hydruino_infrastructure_tests
    host/test_infrastructure.cpp
)
target_link_libraries(hydruino_infrastructure_tests PRIVATE hydruino_host)

add_executable(hydruino_hardware_tests
    host/test_hardware.cpp
)
target_link_libraries(hydruino_hardware_tests PRIVATE hydruino_host)

add_executable(hydruino_serialization_tests
    host/test_serialization.cpp
)
target_link_libraries(hydruino_serialization_tests PRIVATE hydruino_host)

enable_testing()
add_test(NAME hydruino_core_tests COMMAND hydruino_core_tests)
add_test(NAME hydruino_hardening_tests COMMAND hydruino_hardening_tests)
add_test(NAME hydruino_infrastructure_tests COMMAND hydruino_infrastructure_tests)
add_test(NAME hydruino_hardware_tests COMMAND hydruino_hardware_tests)
add_test(NAME hydruino_serialization_tests COMMAND hydruino_serialization_tests)
