cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(fuji VERSION 0.0.0 LANGUAGES CXX)

enable_testing()

include(CTest)

find_package(Vulkan REQUIRED)

set(CORE_SOURCES src/core/core.cpp
                 src/core/instance.cpp
                 src/core/internal/utility/shader_module.cpp
                 src/core/internal/utility/vertex_shader_input_layout.cpp)
set(TEXT_SOURCES src/text/text.cpp)

add_library(fuji src/fuji.cpp ${CORE_SOURCES} ${TEXT_SOURCES})

target_include_directories(fuji PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(fuji PRIVATE Vulkan::Vulkan)

add_subdirectory(test)
add_subdirectory(examples)