cmake_minimum_required(VERSION 3.20)

project(tetris_c LANGUAGES C)

set(APP_VERSION "1.2.0")

set(CMAKE_C_STANDARD 17)

set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(tetris
       src/main.c
       src/piece.c
       src/board.c
       src/game.c
       src/render.c
   )

if (MINGW)
    target_link_options(tetris PRIVATE -static -static-libgcc)
endif()

target_compile_definitions(tetris PRIVATE APP_VERSION="${APP_VERSION}")
