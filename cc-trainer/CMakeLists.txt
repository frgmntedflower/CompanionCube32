cmake_minimum_required(VERSION 3.20)
project(cc_trainer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cc_trainer SHARED
        src/main.cpp
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/backends/imgui_impl_win32.cpp
        lib/imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(cc_trainer
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends
)

target_compile_definitions(cc_trainer PRIVATE CC_TRAINER_EXPORTS)

# Link system libraries
target_link_libraries(cc_trainer
        PRIVATE
        d3d11
        dxgi
        d3dcompiler
        dwmapi
        user32
        gdi32
)
