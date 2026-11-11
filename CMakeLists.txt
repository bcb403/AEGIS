
cmake_minimum_required(VERSION 3.16)

project(AEGIS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(aegis
    src/main.cpp
    src/Aircraft.cpp
    src/Altimeter.cpp
    src/FlightComputer.cpp
    src/FlightDataRecorder.cpp
)

target_include_directories(aegis PRIVATE include)

enable_testing()

add_executable(aegis_tests
    tests/test_flight_computer.cpp
    src/Aircraft.cpp
    src/Altimeter.cpp
    src/FlightComputer.cpp
    src/FlightDataRecorder.cpp
)

target_include_directories(aegis_tests PRIVATE include)

add_test(
    NAME FlightComputerTests
    COMMAND aegis_tests
)
