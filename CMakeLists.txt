cmake_minimum_required(VERSION 3.20)

project(tetris_c VERSION 1.2.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CTest)

add_executable(tetris
    src/main.c
    src/piece.c
    src/board.c
    src/game.c
    src/render.c
)

target_include_directories(tetris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tetris PRIVATE c_std_17)

if (MSVC)
    target_compile_options(tetris PRIVATE /W4)
else()
    target_compile_options(tetris PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

if (MINGW)
    target_link_options(tetris PRIVATE -static -static-libgcc)
endif()

target_compile_definitions(tetris PRIVATE APP_VERSION="${PROJECT_VERSION}")

install(TARGETS tetris
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (BUILD_TESTING)
    add_executable(tetris_tests
        tests/test_core.c
        src/board.c
        src/piece.c
        src/game.c
    )

    target_include_directories(tetris_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_features(tetris_tests PRIVATE c_std_17)

    if (MSVC)
        target_compile_options(tetris_tests PRIVATE /W4)
    else()
        target_compile_options(tetris_tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    endif()

    add_test(NAME tetris_core COMMAND tetris_tests)
endif()
