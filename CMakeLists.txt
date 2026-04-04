cmake_minimum_required(VERSION 3.10)

project(algebra C)

if(NOT CMAKE_BUILD_TYPE) 
    message("No build type specified, defaulting to release.")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Develop")
    add_compile_options(-g -O0)
    message("Making develop build.")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRelease)
    add_compile_options(-O3)
    message("Making release build.")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.2)
link_directories(${YAML_LIBRARY_DIRS})

include_directories(include src ${YAML_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.c")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")

add_executable(${PROJECT_NAME} src/main.c ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${YAML_LIBRARIES} m)
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")