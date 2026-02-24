cmake_minimum_required(VERSION 3.10)
project(TekkenDSL VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force UTF-8 encoding for MSVC
if(MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(tekken_game examples/main.cpp)
add_executable(tekken_example2 examples/example2.cpp)
