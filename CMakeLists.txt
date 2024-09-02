cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(fuji VERSION 0.0.0 LANGUAGES CXX)

enable_testing()

include(CTest)

set(CORE_SOURCES src/core/core.cpp)
set(TEXT_SOURCES src/text/text.cpp)

add_library(fuji src/fuji.cpp ${CORE_SOURCES} ${TEXT_SOURCES})

target_include_directories(fuji PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(test)