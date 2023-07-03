cmake_minimum_required(VERSION 3.19)
project(ctl_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -D_DEBUG")

add_executable(ctl_test main.cpp util_test.cpp)
target_include_directories(ctl_test PRIVATE ${CMAKE_SOURCE_DIR}/../inc/)
