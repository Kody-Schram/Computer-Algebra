add_library(Core STATIC
    axioms.c

    execution/simplify.c
    execution/execute.c
    
    utils/input.c
    utils/log.c
    utils/type_utils.c
    utils/data_types.c
    
    context/config.c
    context/context.c
    context/environment.c
    
    parsing/parser.c
    parsing/parser_utils.c
    
    parsing/codegen/ast.c
    parsing/codegen/lexer.c
    parsing/codegen/tokenizer.c
)

target_link_libraries(Core PRIVATE ${YAML_LIBRARIES})
target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR}/src)