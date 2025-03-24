#include <stdio.h>
#include <assert.h>

#include "../parsing/parser.h" 

void test_basic_operations() {
    assert(parse("1+1") != NULL);
    assert(parse("2-3") != NULL);
    assert(parse("4*5") != NULL);
    assert(parse("6/2") != NULL);
    assert(parse("2^3") != NULL);
    assert(parse("2**3") != NULL); // Ensure ** is rewritten as ^
}

void test_functions() {
    assert(parse("sinx") != NULL);
    assert(parse("cos(x)") != NULL);
    assert(parse("tan(x+1)") != NULL);
    assert(parse("log(x)") != NULL);
    assert(parse("ln(x+2)") != NULL);
}

void test_function_definition() {
    assert(parse("f:x = x+1") != NULL);
    assert(parse("g:x,y = x*y") != NULL);
    assert(parse("h:x = sinx") != NULL);
    //assert(parse("f(2)") != NULL);  // Should recognize previously defined function
}

void test_syntax_errors() {
    assert(parse("1 +") == NULL);
    assert(parse("sin()") == NULL);
    assert(parse("log") == NULL);
    assert(parse("f:x y = x+1") == NULL); // No space allowed between params
}

void test_invalid_operators() {
    // Test cases for invalid binary operators
    assert(parse("sin+") == NULL);  // Should return an error (invalid use of '+')
    assert(parse("sin+)") == NULL); // Should return an error (invalid placement of '+')
    assert(parse("2**") == NULL);   // Should return an error (operator without second operand)
    assert(parse("*3") == NULL);    // Should return an error (operator without first operand)
    assert(parse("^") == NULL);     // Should return an error (standalone operator)
    assert(parse("(2+)3") == NULL); // Should return an error (invalid use of '+')
    assert(parse("sin()*") == NULL);// Should return an error (operator following function call)
    assert(parse("log(2)^") == NULL);// Should return an error (operator at end of expression)
}


void run_parser_tests() {
    test_basic_operations();
    test_functions();
    test_function_definition();
    test_syntax_errors();
    test_invalid_operators();

    printf("All tests passed!\n");
}