# COMPUTER ALGEBRA

*A system which reads mathematical expressions and equations and performs operations on them. These operations include calculus operations and algebraic manipulation.*

## Compiler
---
*Steps*
1. Tokenization (Splits Input into Parts)
2. Lexing (Corrects Input Errors or Ambiguity)
3. Generates AST (Executable Form)
4. Executing

### Syntax
#### Operators
- "+" (addition)
- "-" (subtraction)
- "*" (multiplication)
- "/" (division)
- "^" or "**" (exponentiation)
- "drv" (derivative)
- "int" (integral)


#### Built in Functions
*Parameter brackets will be implied for functions for first term after function*
(eg. sinx+1 => sin(x) + 1)

- sin()
- cos()
- tan()
- csc()
- sec()
- cot()
- log()
- ln()

##### Defining New Functions
Function declaration is indicated by the definition operator (":") followed by the required parameters and the function body. 
_ex. f:x = x+1_ 
_f_ represents the identifier/name, _x_ is the required parameter, and _x+1_ is the function body

This allows the parser to differentiate an identifier and a function declaration. After declaring a function, subsequent uses of the function identifier will automatically be recognized as that function.