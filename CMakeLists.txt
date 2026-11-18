cmake_minimum_required(VERSION 3.20)
project(EscapeFromSevSU LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# raylib скачивается при первой сборке, ставить вручную не нужно
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz)
FetchContent_MakeAvailable(raylib)

# Логика игры отдельной библиотекой: её используют и игра, и тесты
add_library(game_core STATIC
    src/door.cpp
    src/enemy.cpp
    src/flashlight.cpp
    src/game.cpp
    src/grid_map.cpp
    src/item.cpp
    src/level.cpp
    src/level1.cpp
    src/level2.cpp
    src/level3.cpp
    src/player.cpp
    src/record.cpp
    src/renderer.cpp)
target_include_directories(game_core PUBLIC src)
target_link_libraries(game_core PUBLIC raylib)

add_executable(EscapeFromSevSU src/main.cpp)
target_link_libraries(EscapeFromSevSU PRIVATE game_core)
# Игра запускается без консольного окна
set_target_properties(EscapeFromSevSU PROPERTIES WIN32_EXECUTABLE TRUE)
if(MINGW)
    target_link_options(EscapeFromSevSU PRIVATE -static)
endif()

# Папка assets копируется к .exe после каждой сборки
add_custom_command(TARGET EscapeFromSevSU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:EscapeFromSevSU>/assets)

enable_testing()
add_executable(game_tests tests/game_tests.cpp)
target_link_libraries(game_tests PRIVATE game_core)
if(MINGW)
    target_link_options(game_tests PRIVATE -static)
endif()
target_compile_definitions(game_tests PRIVATE
    MAPS_DIR="${CMAKE_SOURCE_DIR}/assets/maps/")
add_test(NAME game_tests COMMAND game_tests)
