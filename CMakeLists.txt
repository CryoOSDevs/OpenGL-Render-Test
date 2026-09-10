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
    glad
    GIT_REPOSITORY https://github.com/Dav1dDe/glad.git
    GIT_TAG v2.0.4
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

FetchContent_MakeAvailable(glfw glm glad imgui imguizmo)

# GLAD generates the OpenGL loader and exposes the CMake target.
# GLFW adds its own imported target; the app links against the generated OpenGL libs.
add_library(glad_lib INTERFACE)
target_include_directories(glad_lib INTERFACE ${glad_SOURCE_DIR}/include)
if (TARGET glad::glad)
    target_link_libraries(glad_lib INTERFACE glad::glad)
endif()

# ImGui sources used by the app.
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/imgui_impl_opengl3.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC glfw)

# ImGuizmo sources.
add_library(imguizmo_lib STATIC
    ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)
target_include_directories(imguizmo_lib PUBLIC ${imguizmo_SOURCE_DIR})
target_link_libraries(imguizmo_lib PUBLIC imgui_lib glm)

# Application.
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/panels/*.cpp")

add_executable(LauraEditor ${APP_SOURCES})

target_include_directories(LauraEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
    ${glad_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imguizmo_SOURCE_DIR}
)

target_link_libraries(LauraEditor PRIVATE
    glfw
    glm
    glad_lib
    imgui_lib
    imguizmo_lib
    OpenGL::GL
)

if (MSVC)
    target_compile_options(LauraEditor PRIVATE /W4)
else()
    target_compile_options(LauraEditor PRIVATE -Wall -Wextra -pedantic)
endif()

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.vert" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.frag" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")
