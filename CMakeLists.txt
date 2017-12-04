cmake_minimum_required(VERSION 3.5.1)
project(rt_timer_benchmark)

set(CMAKE_C_STANDARD 99)

add_executable(rt_timer_benchmark
        main.c)

set(CMAKE_VERBOSE_MAKEFILE ON)

target_link_libraries(rt_timer_benchmark "-lrt")
