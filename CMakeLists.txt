cmake_minimum_required(VERSION 3.5)

project(GraphicsHeaven
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)


# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)

FetchContent_MakeAvailable(glm)


# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.0
)

FetchContent_MakeAvailable(imgui)



# GLAD

add_library(glad STATIC
    external/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)


# ImGui library

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC
    glfw
    glad
)

# OpenGL

find_package(OpenGL REQUIRED)


# Main executable

file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE PROJECT_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

add_executable(GraphicsHeaven
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

source_group(
    TREE "${CMAKE_SOURCE_DIR}"
    FILES
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
)


# Include directories

target_include_directories(GraphicsHeaven PRIVATE
    include

    external/cgltf
    external/stb
)


# Libraries

target_link_libraries(GraphicsHeaven PRIVATE
    glad
    glfw
    glm::glm
    imgui_lib
    OpenGL::GL
)


# Windows settings

if (WIN32)
    target_compile_definitions(GraphicsHeaven PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()


# Visual Studio Config
set_target_properties(GraphicsHeaven PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:GraphicsHeaven>"
)


# Runtime assets

add_custom_command(TARGET GraphicsHeaven POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:GraphicsHeaven>/assets"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:GraphicsHeaven>/shaders"
)