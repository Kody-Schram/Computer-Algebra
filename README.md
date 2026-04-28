# COMPUTER ALGEBRA

*A system which parses and executes mathematical expressions and equations.*

## Dependencies
- [libyaml](https://github.com/yaml/libyaml)


## Configuration

Configuration through YAML files is supported. [Example config](config.example.yaml) provides documentation on the different configurations.



## Compiler

*Steps*
1. Tokenization (Splits input into parts)
2. Normalizing (Attempts to correct input errors or ambiguities)
3. Generates AST (Executable form)
5. Executing



## Syntax

### Operators

- "+" Addition
- "-" Subtraction
- "*" Multiplication
- "/" Division
- "^" or "**" Exponentiation
- "->" Mapping
- ":" Assignment


### Declaring Variables
Variable declaration is structured as follows:
*(value): (variable name)*


### Defining New Functions
Function declaration is structured as follows:

*(function name) : (list of parameters seperated with ","s) -> (function definition)*

This allows the parser to differentiate an identifier and a function declaration. After declaring a function, subsequent uses of the function identifier will automatically be recognized as that function.

*All functions and variables declared by the user get added to the global environment of the system and can then be used again on subsequent lines*

### Implicit Syntax
Some implicit assumptions are made by the parser in the interest of a simpler user exprience.

#### Function Calls
Function calls can be implicit. When a function call is identified with no parenthesis, the call will consume all tokens until the first user defined operation.

Ex. 
'''
f:x->2x
fa*3a becomes f(a)*3a
'''

#### Multiplication
Implicit multiplication is supported in some cases
Ex.
'''
2x becomes 2*x
2sin(x) becomes 2*sin(x)
3(x-1) becomes 3*(x-1)
'''
