add_library(Modules STATIC
    algebra.c
    utils.c
)

target_link_libraries(Modules PRIVATE Core)
target_include_directories(Modules PUBLIC ${CMAKE_SOURCE_DIR}/src)