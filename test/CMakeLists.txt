project(fuji_test)

find_package(GTest REQUIRED)

function(add_unittest test_suit)
    string(REPLACE "/" "_" test_suit_name "${test_suit}")
    message(${test_suit_name})
    add_executable(run_${test_suit_name} ${test_suit}.cpp)
    target_include_directories(run_${test_suit_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
    target_link_libraries(run_${test_suit_name} PRIVATE GTest::gtest GTest::gmock Vulkan::Vulkan ${GTEST_MAIN_LIBRARIES} fuji)
    add_test(NAME ${test_suit_name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_${test_suit_name})
endfunction()

function(add_shader_resource shader_name suffix targets)
    string(TOUPPER ${shader_name}_${suffix}_SPV_FILE file_path_macro)
    set(output_file ${shader_name}_${suffix}.spv)
    add_custom_target(${output_file} 
        glslangValidator 
            -V ${CMAKE_CURRENT_SOURCE_DIR}/resource/shader/${shader_name}.${suffix}
            -o ${CMAKE_CURRENT_BINARY_DIR}/${output_file})
    
    foreach(target IN LISTS targets)
        target_compile_definitions(run_${target} PRIVATE ${file_path_macro}="${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
        add_dependencies(run_${target} ${output_file})
    endforeach()
endfunction()

add_unittest(fuji_test)
add_unittest(core/internal/utility/shader_module_test)
add_unittest(core/internal/utility/vertex_shader_input_layout_test)
add_unittest(core/internal/utility/shader_stage_flow_test)

set(UNITTEST_TARGETS 
    fuji_test 
    core_internal_utility_shader_module_test 
    core_internal_utility_vertex_shader_input_layout_test
    core_internal_utility_shader_stage_flow_test)

add_shader_resource(test vert "${UNITTEST_TARGETS}")
add_shader_resource(test frag "${UNITTEST_TARGETS}")