cmake_minimum_required(VERSION 3.10)
project(ComputerAlgebra C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

include_directories(include src ${YAML_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(main ${SOURCES})

target_link_libraries(main ${YAML_LIBRARIES})