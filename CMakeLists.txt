cmake_minimum_required(VERSION 3.25)

project(MyProject LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Build options
#
option(ENABLE_WERROR "Treat warnings as errors" OFF)
option(ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

# Warnings: always on, applied privately to our own targets (not propagated
# to GoogleTest or other consumers).
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    $<$<BOOL:${ENABLE_WERROR}>:-Werror>
)

# Sanitizers: opt-in via -DENABLE_SANITIZERS=ON. Must appear on both the
# compile and link lines, so this interface target carries both.
add_library(project_sanitizers INTERFACE)
if(ENABLE_SANITIZERS)
    target_compile_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined
    )
endif()

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

target_link_libraries(lib
    PRIVATE
        project_warnings
        project_sanitizers
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
        project_warnings
        project_sanitizers
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
        project_warnings
        project_sanitizers
)

include(GoogleTest)
gtest_discover_tests(unit_tests)