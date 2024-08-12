cmake_minimum_required(VERSION 3.10)

project(Add)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(add_mul add_mul.cpp)
add_executable(add_mul_verify add_mul_verify.cpp)

target_include_directories(add_mul PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(add_mul_verify PRIVATE ${CMAKE_SOURCE_DIR})
