add_executable(character_a main.cpp ${FUJI_EXAMPLES_COMMON_SOURCES})
target_link_libraries(character_a glfw vulkan)
target_include_directories(character_a PRIVATE ${FUJI_EXAMPLES_COMMON_INCLUDE})