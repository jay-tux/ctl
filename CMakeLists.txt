cmake_minimum_required(VERSION 3.19)
project(ctl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -D_DEBUG")

add_executable(ctl main.cpp src/graph/ts.cpp src/graph/graph_reader.cpp src/formula/formula.cpp src/formula/formula_parser.cpp)
target_include_directories(ctl PRIVATE ${CMAKE_SOURCE_DIR}/inc/)