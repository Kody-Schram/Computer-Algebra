# Algorithms Used

## Parsing
*Algorithms used within parsing of input*

The tokenizer, lexer, and parsing is all custom, no algorithms or common implementations were followed.

- Shunting Yard to generate RPN (Reverse Polish Notation):
  - Implemented src/parser/codegen/ast.c in shuntingYard()
  - [Source](https://en.wikipedia.org/wiki/Shunting_yard_algorithm)

## Execution
### Algebra

- Gröbner Bases for algebraic simplification
  - Implemented at src/execution/algebra/algebra.c in simplifyRecur()
  - [Source](https://mathweb.ucsd.edu/~helton/BILLSPAPERSscanned/HSW98.pdf)

### Calculus
- Risch/Risch-Norman for symbolic integration
  - Not implemented yet
  - Research not done yet