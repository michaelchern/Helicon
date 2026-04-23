cmake_minimum_required(VERSION 3.20)

project(Helicon VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(helicon STATIC
    src/helicon.c
)

target_include_directories(helicon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(helicon_tests
    tests/helicon_test.c
)

target_link_libraries(helicon_tests
    PRIVATE
        helicon
)

add_test(
    NAME helicon_tests
    COMMAND helicon_tests
)
