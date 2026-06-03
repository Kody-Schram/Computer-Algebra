add_library(Core STATIC
    utils/input.c
    utils/log.c
    utils/expr_utils.c
    
    context/config.c
    context/context.c
    context/environment.c
	context/registry.c
    
    parsing/parser_utils.c
    parsing/parser.c
    
    parsing/codegen/ast.c
    parsing/codegen/normalizer.c
    parsing/codegen/tokenizer.c
    
	execution/executor.c
    execution/simplify.c
    
	primitives/integers.c
)

target_link_libraries(Core PRIVATE ${YAML_LIBRARIES})
target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR}/src)
