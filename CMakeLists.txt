cmake_minimum_required(VERSION 3.10)

project(player)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavdevice
        libavformat
        libavcodec
        libavutil
        libavfilter
        libswresample
        libswscale
        )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(SDL2_IMAGE REQUIRED)

include_directories(${SDL2_IMAGE_INCLUDE_DIR})
include_directories(${SDL2_TTF_INCLUDE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/includes")

set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
file(GLOB SOURCE
        "${SOURCE_DIR}/*.cpp"
        "${SOURCE_DIR}/**/*.cpp"
        "${SOURCE_DIR}/*.cc"
        "${SOURCE_DIR}/**/*.cc"
        )
add_executable(player ${SOURCE})

target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FFMPEG SDL2::SDL2 ${SDL2_TTF_LIBRARY} ${SDL2_IMAGE_LIBRARY})
