cmake_minimum_required(VERSION 3.29)
project(ionEditor)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded" CACHE STRING "Select MSVC runtime library.")
endif()

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE
        SRC_FILES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_executable(ionEditor ${SRC_FILES})

add_subdirectory(${CMAKE_SOURCE_DIR}/../ionEngine ${CMAKE_BINARY_DIR}/ionEngine)
target_link_libraries(ionEditor PRIVATE ionEngine)