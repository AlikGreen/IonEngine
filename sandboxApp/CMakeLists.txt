cmake_minimum_required(VERSION 3.29)
project(sandboxApp)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded" CACHE STRING "Select MSVC runtime library.")
endif()

set(CMAKE_CXX_STANDARD 23)

add_executable(sandboxApp
        main.cpp
        gameSystem.cpp
        gameSystem.h)

add_subdirectory(${CMAKE_SOURCE_DIR}/../ionEngine ${CMAKE_BINARY_DIR}/ionEngine)
target_link_libraries(sandboxApp PRIVATE ionEngine)