cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(algebra C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE) 
    message("No build type specified, defaulting to release.")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Develop")
    add_compile_options(-g3 -O0)
    message("Making develop build.")
endif()

# Use for gprof profiling
if(CMAKE_BUILD_TYPE STREQUAL "Profile")
    add_compile_options(-pg -O2)
    add_link_options(-pg)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRelease)
    add_compile_options(-s -O3)
    message("Making release build.")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

add_subdirectory(src/core)
add_subdirectory(src/modules)
add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Core
    Modules
    ${YAML_LIBRARIES}
    m
)