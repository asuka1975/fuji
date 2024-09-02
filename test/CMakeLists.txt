project(fuji_test)

find_package(GTest REQUIRED)

function(add_unittest test_suit)
    message(${test_suit})
    add_executable(run_${test_suit} ${test_suit}.cpp)
    target_include_directories(run_${test_suit} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${GTEST_INCLUDE_DIRS})
    target_link_libraries(run_${test_suit} PRIVATE ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    add_test(NAME run_${test_suit} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_${test_suit})
endfunction()

add_unittest(fuji_test)