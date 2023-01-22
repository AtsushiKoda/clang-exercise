cmake_minimum_required(VERSION 3.17)

project(webclient)

set(CMAKE_CXX_FLAGS "-Wall -W -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "-Wall -W -fprofile-arcs -ftest-coverage")

add_executable(webclienttest cpputest.cpp)

target_link_libraries(webclienttest CppUTest)
target_link_libraries(webclienttest CppUTestExt)
