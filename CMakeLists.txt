cmake_minimum_required(VERSION 3.20)

project(VWBeetleDigitalCluster LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_executable(
    vw_beetle_cluster
    src/main.c
    src/display.c
    src/simulator.c
    src/cluster_state.c
)

target_include_directories(
    vw_beetle_cluster
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    vw_beetle_cluster
    PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
)

target_compile_options(
    vw_beetle_cluster
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_custom_command(
    TARGET vw_beetle_cluster
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:vw_beetle_cluster>/assets
)