cmake_minimum_required(VERSION 3.7)
project(rt_timer_benchmark)

set(CMAKE_CXX_STANDARD 11)

add_executable(rt_timer_benchmark
        main.cpp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lrt")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrt")
