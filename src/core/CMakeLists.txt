add_library(Core STATIC
    execution/execute.c
    
    utils/input.c
    utils/log.c
    utils/types.c
    
    utils/context/config.c
    utils/context/context.c
    utils/context/environment.c
    
    parsing/parser.c
    parsing/parserUtils.c
    
    parsing/codegen/ast.c
    parsing/codegen/lexer.c
    parsing/codegen/tokenizer.c
)

target_link_libraries(Core PRIVATE ${YAML_LIBRARIES})
target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR}/src)