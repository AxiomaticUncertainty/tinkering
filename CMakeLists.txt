cmake_minimum_required(VERSION 3.25)

project(MyProject LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Main library
#
add_library(lib
    src/reader.cpp
    src/table.cpp
    src/writer.cpp
)

target_include_directories(lib
    PUBLIC
        include
)

#
# Main executable
#
add_executable(main
    src/main.cpp
)

target_link_libraries(main
    PRIVATE
        lib
)

#
# GoogleTest
#
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

#
# Tests
#
add_executable(unit_tests
    tests/table.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)