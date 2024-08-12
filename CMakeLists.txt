cmake_minimum_required(VERSION 3.10)

project(Add)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mul mul.cpp)
add_executable(mul-verify mul-verify.cpp)

target_include_directories(mul PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(mul-verify PRIVATE ${CMAKE_SOURCE_DIR})
