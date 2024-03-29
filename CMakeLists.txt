cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_COVERAGE "Build coverage" OFF)

set(
        HUNTER_CACHE_SERVERS
        "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
        CACHE STRING "Default cache server"
)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.314.tar.gz"
        SHA1 "95c47c92f68edb091b5d6d18924baabe02a6962a"
)

project(builder)
hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)
hunter_add_package(Boost COMPONENTS program_options filesystem system regex)
hunter_add_package(Async++)
find_package(Async++ CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED program_options filesystem system regex)

add_library(builder STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/my_cmake.cpp
        )
add_library(process INTERFACE
        )
add_executable(main
        ${CMAKE_CURRENT_SOURCE_DIR}/builder/builder.cpp
        )

target_include_directories(process
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/process/include
        )
target_include_directories(builder
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_link_libraries(builder process Async++::Async++
        Boost::program_options Boost::filesystem Boost::system Boost::regex)

    add_executable(tests

            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp
            )

target_link_libraries(
        tests
        builder
        GTest::gtest_main
)
enable_testing()
add_test(NAME unit_tests COMMAND tests)
target_link_libraries(main builder)
