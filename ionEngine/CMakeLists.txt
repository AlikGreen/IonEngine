cmake_minimum_required(VERSION 3.29)
project(ionEngine)
set(CMAKE_CXX_STANDARD 23)

set(TINYGLTF_HEADER_ONLY OFF CACHE BOOL "Build tinygltf as a library" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/tinygltf)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/miniaudio)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/urhi)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/entis)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/coral)

file(GLOB_RECURSE
        SRC_FILES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(ionEngine STATIC ${SRC_FILES})

target_link_options(ionEngine
        INTERFACE
        -static
        -static-libgcc
        -static-libstdc++
)

target_compile_definitions(ionEngine PRIVATE GLFW_INCLUDE_NONE)

set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
target_precompile_headers(ionEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h")

target_include_directories(ionEngine PUBLIC src/ include/ )
target_link_libraries(ionEngine PUBLIC
        glfw
        tinygltf
        urhi
        entis
        urhi::imgui
        miniaudio
        Coral.Native)