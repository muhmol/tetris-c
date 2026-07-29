cmake_minimum_required(VERSION 3.20)

project(tetris_c LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(tetris src/main.c)
if (MINGW)
    target_link_options(tetris PRIVATE -static -static-libgcc)
endif()
