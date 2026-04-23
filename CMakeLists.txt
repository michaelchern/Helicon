cmake_minimum_required(VERSION 3.20)

project(Helicon VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HELICON_BUILD_EXAMPLES "Build Helicon examples" ON)

find_package(Vulkan REQUIRED)

add_library(helicon STATIC
    src/rhi.cpp
    src/render_graph.cpp
    src/backends/vulkan/vulkan_backend.cpp
)

target_include_directories(helicon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(helicon
    PRIVATE
        Vulkan::Vulkan
)

target_compile_features(helicon
    PUBLIC
        cxx_std_20
)

if(WIN32)
    target_compile_definitions(helicon
        PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

enable_testing()

add_executable(helicon_tests
    tests/helicon_smoke_test.cpp
)

target_link_libraries(helicon_tests
    PRIVATE
        helicon
)

add_test(
    NAME helicon_tests
    COMMAND helicon_tests
)

add_executable(helicon_render_graph_tests
    tests/render_graph_test.cpp
)

target_link_libraries(helicon_render_graph_tests
    PRIVATE
        helicon
)

add_test(
    NAME helicon_render_graph_tests
    COMMAND helicon_render_graph_tests
)

if(HELICON_BUILD_EXAMPLES)
    add_executable(helicon_triangle_graph
        examples/triangle_graph/main.cpp
    )

    target_link_libraries(helicon_triangle_graph
        PRIVATE
            helicon
    )
endif()
