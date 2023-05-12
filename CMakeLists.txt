project(test)
cmake_minimum_required(VERSION 3.19)
set(CXX_STANDARD 20)
add_executable(test test.cpp)
include_directories(lib/googletest/googletest/include)
target_link_libraries(test mysqlcppconn gtest gtest_main)

