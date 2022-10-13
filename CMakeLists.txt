cmake_minimum_required(VERSION 3.23)
project(cycled_view)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest)

add_executable(tests test/cycled_view.cpp)
target_link_libraries(tests gtest gtest_main)
target_include_directories(tests PRIVATE include)
target_compile_options(tests PRIVATE -fconcepts-diagnostics-depth=5)

gtest_discover_tests(tests)
