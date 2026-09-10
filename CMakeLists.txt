cmake_minimum_required(VERSION 3.16)

project(LauraEditor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG master
)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.8.18
)

FetchContent_MakeAvailable(glfw glm)
FetchContent_Populate(imgui)
FetchContent_Populate(imguizmo)
FetchContent_Populate(tinygltf)
FetchContent_GetProperties(imgui)
FetchContent_GetProperties(imguizmo)
FetchContent_GetProperties(tinygltf)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(OpenGL REQUIRED)
set(GLAD_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated_glad")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m glad --generator c --spec gl --out-path "${GLAD_GENERATED_DIR}" --api gl=3.3 --profile core --quiet
    RESULT_VARIABLE GLAD_GENERATE_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)
if(NOT GLAD_GENERATE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate glad loader via Python.")
endif()

add_library(glad_loader STATIC "${GLAD_GENERATED_DIR}/src/glad.c")
target_include_directories(glad_loader PUBLIC "${GLAD_GENERATED_DIR}/include")

# ImGui sources used by the app.
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC glfw)

# ImGuizmo sources.
add_library(imguizmo_lib STATIC
    ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp
)
target_include_directories(imguizmo_lib PUBLIC
    ${imguizmo_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
)
target_link_libraries(imguizmo_lib PUBLIC imgui_lib glm)

# Application.
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/panels/*.cpp")

add_executable(LauraEditor ${APP_SOURCES})

target_include_directories(LauraEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
    ${GLAD_GENERATED_DIR}/include
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imguizmo_SOURCE_DIR}/src
    ${tinygltf_SOURCE_DIR}
)

target_link_libraries(LauraEditor PRIVATE
    glfw
    glm
    glad_loader
    imgui_lib
    imguizmo_lib
    tinygltf
    OpenGL::GL
)

if (MSVC)
    target_compile_options(LauraEditor PRIVATE /W4)
else()
    target_compile_options(LauraEditor PRIVATE -Wall -Wextra -pedantic)
endif()

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.vert" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.frag" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")
