cmake_minimum_required(VERSION 3.20)
project(cc_inject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc -mthreads")

add_executable(cc_inject main.cpp)