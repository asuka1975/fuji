cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(fuji VERSION 0.0.0 LANGUAGES CXX)

enable_testing()

include(CTest)

add_library(fuji src/fuji.cpp)

add_subdirectory(test)