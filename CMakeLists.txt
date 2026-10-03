cmake_minimum_required(VERSION 3.16)
project(physics LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(physics
    src/main.cpp
    include/imgui/imgui.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
)

target_compile_features(physics PRIVATE cxx_std_17)
target_include_directories(physics PRIVATE
    include/imgui
    include/imgui/backends
)

if(TARGET glfw)
    target_link_libraries(physics PRIVATE glfw)
elseif(TARGET glfw3::glfw)
    target_link_libraries(physics PRIVATE glfw3::glfw)
else()
    message(FATAL_ERROR "The glfw3 package did not provide a GLFW target")
endif()

target_link_libraries(physics PRIVATE OpenGL::GL GLUT::GLUT)