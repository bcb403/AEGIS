
cmake_minimum_required(VERSION 3.16)

project(AEGIS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(aegis
    src/main.cpp
    src/Aircraft.cpp
)

target_include_directories(aegis PRIVATE include)
