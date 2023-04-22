cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project("fastchan" LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(fastchan include/spsc.hpp include/mpsc.hpp)
set_target_properties(fastchan PROPERTIES LINKER_LANGUAGE CXX)

find_package(Threads)


option(BENCHMARK_ENABLE_TESTING "run benchmarks" OFF)

FILE(GLOB tests ${PROJECT_SOURCE_DIR}/test/*)
FOREACH (test ${tests})
    get_filename_component(test_name ${test} NAME)
    message("Adding test: " ${test_name})
    add_executable(${test_name} ${PROJECT_SOURCE_DIR}/test/${test_name})
    target_link_libraries(${test_name} ${CMAKE_THREAD_LIBS_INIT})
    target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
    add_test(${test_name} ${test_name})
    set_property(TEST ${test_name} PROPERTY LABELS "test")
ENDFOREACH ()


if (BENCHMARK_ENABLE_TESTING)
    include(cmake/CPM.cmake)
    CPMUsePackageLock(package-lock.cmake)

    CPMAddPackage( NAME benchmark GITHUB_REPOSITORY google/benchmark VERSION 1.5.2 OPTIONS "BENCHMARK_ENABLE_TESTING Off")

    if(benchmark_ADDED)
        # enable c++11 to avoid compilation errors
        set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
    endif()

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    CPMAddPackage( NAME Boost VERSION 1.80.0 GITHUB_REPOSITORY "boostorg/boost" GIT_TAG "boost-1.80.0")


    FILE(GLOB tests ${PROJECT_SOURCE_DIR}/bench/*)
    FOREACH (test ${tests})
        get_filename_component(test_name ${test} NAME)
        message("Adding bench: " ${test_name})
        add_executable(${test_name} ${PROJECT_SOURCE_DIR}/bench/${test_name})
        target_link_libraries(${test_name} ${CMAKE_THREAD_LIBS_INIT})
        target_link_libraries(${test_name} PRIVATE Boost::lockfree)
        target_link_libraries(${test_name} PRIVATE benchmark::benchmark)
        target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
        add_test(${test_name} ${test_name})
        set_property(TEST ${test_name} PROPERTY LABELS "bench")
        list(APPEND ignore_tests ${test_name})
    ENDFOREACH ()

    set (CMAKE_CTEST_ARGUMENTS "-L;test")
    add_custom_target(bench COMMAND ctest -L bench -V)
endif()

