add_executable(character_a main.cpp ${FUJI_EXAMPLES_COMMON_SOURCES})
target_link_libraries(character_a glfw vulkan)
target_include_directories(character_a PRIVATE ${FUJI_EXAMPLES_COMMON_INCLUDE})

add_custom_target(texture_vert.spv 
    glslangValidator 
        -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/texture.vert 
        -o ${CMAKE_CURRENT_BINARY_DIR}/texture_vert.spv)
add_custom_target(texture_frag.spv 
    glslangValidator 
        -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/texture.frag 
        -o ${CMAKE_CURRENT_BINARY_DIR}/texture_frag.spv)

target_compile_definitions(character_a PRIVATE TEXTURE_VERT_SPV_FILE="${CMAKE_CURRENT_BINARY_DIR}/texture_vert.spv")
target_compile_definitions(character_a PRIVATE TEXTURE_FRAG_SPV_FILE="${CMAKE_CURRENT_BINARY_DIR}/texture_frag.spv")

add_dependencies(character_a texture_vert.spv texture_frag.spv)