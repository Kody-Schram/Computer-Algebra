# COMPUTER ALGEBRA

*A system which reads mathematical expressions and equations and performs operations on them. These operations include calculus operations and algebraic manipulation.*

## Compiler
---
*Steps*
1. Tokenization and Lexing
2. Building AST
3. Executing

### Tokenization
Takes in a string and parses it into a list of tokens. Some expressions will be rewritten into a stardardized way to represent expressions (ex. ** -> ^ for exponents).

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
Function declaration is indicated by : operator. ":" allows the parser to differentiate an identifier and a function declaration. After declaring a function, subsequent uses of the function identifier will automatically be recognized as that function.

*eg. f:x = x+1*