cmake_minimum_required(VERSION 3.10)

project(algebra C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

include_directories(include src ${YAML_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(algebra ${SOURCES})

target_link_libraries(algebra ${YAML_LIBRARIES})