cmake_minimum_required(VERSION 3.10)

project(algebra C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

include_directories(include src ${YAML_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")

target_link_libraries(${PROJECT_NAME} ${YAML_LIBRARIES})