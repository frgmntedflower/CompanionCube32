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
        lib/imgui/backends/imgui_impl_dx9.cpp
        lib/minhook/src/buffer.c
        lib/minhook/src/hook.c
        lib/minhook/src/trampoline.c
        lib/minhook/src/hde/hde32.c
)

target_include_directories(cc_trainer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/minhook/include
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(cc_trainer PRIVATE
            -static-libgcc
            -static-libstdc++
            -mthreads
    )
    target_link_options(cc_trainer PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
    )
endif()

target_link_libraries(cc_trainer PUBLIC
        user32
        d3d9
        dwmapi
)
