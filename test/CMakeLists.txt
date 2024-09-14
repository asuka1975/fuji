project(fuji_test)

find_package(GTest REQUIRED)

function(add_unittest test_suit)
    string(REPLACE "/" "_" test_suit_name "${test_suit}")
    message(${test_suit_name})
    add_executable(run_${test_suit_name} ${test_suit}.cpp)
    target_include_directories(run_${test_suit_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
    target_link_libraries(run_${test_suit_name} PRIVATE GTest::gtest GTest::gmock Vulkan::Vulkan ${GTEST_MAIN_LIBRARIES} fuji)
    add_shader_resource(run_${test_suit_name} test vert)
    add_test(NAME run_${test_suit_name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_${test_suit_name})
endfunction()

function(add_shader_resource target shader_name suffix)
    string(TOUPPER ${shader_name}_${suffix}_SPV_FILE file_path_macro)
    set(output_file ${target}_${shader_name}_${suffix}.spv)
    add_custom_target(${output_file} 
        glslangValidator 
            -V ${CMAKE_CURRENT_SOURCE_DIR}/resource/shader/${shader_name}.${suffix}
            -o ${CMAKE_CURRENT_BINARY_DIR}/${output_file})
    target_compile_definitions(${target} PRIVATE ${file_path_macro}="${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
    add_dependencies(${target} ${output_file})
endfunction()

add_unittest(fuji_test)
add_unittest(core/internal/utility/shader_module_test)
