cmake_minimum_required(VERSION 3.16)
project(sv_common LANGUAGES CXX)

file(GLOB_RECURSE SV_COMMON_SOURCES "*.cpp" "*.h")

add_library(sv_common STATIC ${SV_COMMON_SOURCES})

target_include_directories(sv_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)