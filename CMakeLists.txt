cmake_minimum_required(VERSION 3.29)
project(ionEngine)
set(CMAKE_CXX_STANDARD 23)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

set(TINYGLTF_HEADER_ONLY OFF CACHE BOOL "Build tinygltf as a library" FORCE)
add_subdirectory(${DEPS_DIR}/tinygltf)
add_subdirectory(${DEPS_DIR}/miniaudio)
set(DILIGENT_NO_SUPERRESOLUTION ON CACHE BOOL "" FORCE)
add_subdirectory(${DEPS_DIR}/DiligentCore)
add_subdirectory(${DEPS_DIR}/entis)
add_subdirectory(${DEPS_DIR}/coral)
add_subdirectory(${DEPS_DIR}/jsoncons)
add_subdirectory(${DEPS_DIR}/meshoptimizer)
add_subdirectory(${DEPS_DIR}/clogr)
add_subdirectory(${DEPS_DIR}/glfw)
add_subdirectory(${DEPS_DIR}/glm)
add_subdirectory(${DEPS_DIR}/imguizmo)
add_subdirectory(${DEPS_DIR}/ionsl)

set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${DEPS_DIR}/spirv-reflect)

set(IMGUI_DIR ${DEPS_DIR}/imgui)

add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(imguizmo PUBLIC imgui)

# Add diligent tools after creating imgui target
add_subdirectory(${DEPS_DIR}/DiligentTools)

set(DXC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/dxc")

add_library(dxcompiler SHARED IMPORTED GLOBAL)
target_include_directories(dxcompiler INTERFACE "${DXC_DIR}/inc")

if(WIN32)
    set_target_properties(dxcompiler PROPERTIES
            IMPORTED_IMPLIB "${DXC_DIR}/lib/x64/dxcompiler.lib"
            IMPORTED_LOCATION "${DXC_DIR}/bin/x64/dxcompiler.dll"
    )
    set(DXC_RUNTIME_FILES
            "${DXC_DIR}/bin/x64/dxcompiler.dll"
            "${DXC_DIR}/bin/x64/dxil.dll"
    )
else()
    set_target_properties(dxcompiler PROPERTIES
            IMPORTED_LOCATION "${DXC_DIR}/lib/libdxcompiler.so"
            IMPORTED_NO_SONAME TRUE
    )
    set(DXC_RUNTIME_FILES
            "${DXC_DIR}/lib/libdxcompiler.so"
            "${DXC_DIR}/lib/libdxil.so"
    )
endif()


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

set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
target_precompile_headers(ionEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/common.h")

target_compile_definitions(ionEngine PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(ionEngine PUBLIC src/ include/ )

target_include_directories(ionEngine PUBLIC
        ${DEPS_DIR}/DiligentCore/Graphics/GraphicsEngine/interface
        ${DEPS_DIR}/DiligentCore/Graphics/GraphicsEngineVulkan/interface
        ${DEPS_DIR}/DiligentCore/Graphics/GraphicsEngineOpenGL/interface
        ${DEPS_DIR}/DiligentCore/Common/interface
)

target_link_libraries(ionEngine PUBLIC
        Diligent-GraphicsEngineVk-static
        Diligent-Imgui
        Diligent-GraphicsAccessories
        imgui
        imguizmo
        glm::glm
        glfw
        tinygltf
        entis
        clogr
        miniaudio
        jsoncons
        Coral.Native
        meshoptimizer
        dxcompiler
        spirv-reflect-static
        ionsl)