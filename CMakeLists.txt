cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project("fastchan" LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(fastchan include/fastchan.hpp)
set_target_properties(fastchan PROPERTIES LINKER_LANGUAGE CXX)

find_package(Threads REQUIRED)

FILE(GLOB tests ${PROJECT_SOURCE_DIR}/test/*)
FOREACH (test ${tests})
    get_filename_component(test_name ${test} NAME)
    message("Adding test: " ${test_name})
    add_executable(${test_name} ${PROJECT_SOURCE_DIR}/test/${test_name})
    target_link_libraries(${test_name} PRIVATE Threads::Threads)
    target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
    add_test(${test_name} ${test_name})
    if ("${test_name}" MATCHES "bench")
        set_property(TEST ${test_name} PROPERTY LABELS "bench")
        list(APPEND ignore_tests ${test_name})
    else()
        set_property(TEST ${test_name} PROPERTY LABELS "test")
    endif()
ENDFOREACH ()

set (CMAKE_CTEST_ARGUMENTS "-L;test")

add_custom_target(bench COMMAND ctest -L bench -V)
