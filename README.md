# This repository has moved to Codeberg: https://codeberg.org/KodySchram/Computer-Algebra

# COMPUTER ALGEBRA

*A system which parses and executes mathematical expressions and equations.*

## Dependencies
- [libyaml](https://github.com/yaml/libyaml)

## Installation
1. Install required dependencies (some OSes such as Ubuntu will require *-dev packages or similar to enable proper linking)

2. Ensure you have CMake installed on your machine

3. Clone the repo and run this in the project directory
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-march=native" ..
cmake --build build
```

**Note: -DCMAKE_C_FLAGS="-march=native" is optional for better local performance. Not recommended if you plan to distribute or run on any machines other than the one building the program**


## Configuration

Configuration through YAML file is supported.
[Example config](config.example.yaml) provides documentation on the different configurations. 
The configuration path defaults to `~/.config/algebra/config.yaml` unless overridden when running from the terminal
`Ex: algebra /home/(user)/(custom override path)`


## Usage

### Syntax

#### Operators

- "+" Addition
- "-" Subtraction
- "*" Multiplication
- "/" Division
- "^" or "**" Exponentiation
- "->" Mapping
- ":" Assignment


#### Declaring Variables
Variable declaration is structured as follows:
*(variable name): (expression)*


#### Defining New Functions
Function declaration is structured as follows:

*(function name) : (parameters) -> (expression)*

This allows the parser to differentiate an identifier and a function declaration. After declaring a function, subsequent uses of the function identifier will automatically be recognized as that function.

*All functions and variables declared by the user get added to the global environment of the system and can then be used again on subsequent lines*

### Implicit Syntax
Some implicit assumptions are made by the parser in the interest of a simpler user exprience.

#### Function Calls
Function calls can be implicit. When a function call is identified with no parenthesis, the call will consume all tokens until the first user defined operation.

Ex. 
```
f:x->2x
fa*3a becomes f(a)*3a
```

#### Multiplication
Implicit multiplication is supported in some cases
Ex.
```
2x becomes 2*x
2sin(x) becomes 2*sin(x)
3(x-1) becomes 3*(x-1)
```

## Program Details
*Information on algorithms used and sources of research can be found in [ALGORITHMS.md](ALGORITHMS.md)*

### Parsing
1. Tokenization (Splits input into parts)
2. Normalizing (Attempts to correct input errors or ambiguities)
3. AST Generation (Converts input into the representation the rest of the system understands)

### Execution
1. Symbol Resolution (Subprocess which resolve variables or functions into their definitions)
2. Simplification (Simplifies AST, flattening nested operations, combining like terms, etc.)
3. Evaluation

## AI Disclosure
I am ashamed to say that, up until release 1.1.0, generative AI was used during development on this project. I cannot in good conscious continue to use or support generative AI due to environmental, economic, and social concerns. I have since stopped using it entirely.

Prior to this change, AI was used largely for debugging or quick brainstorming and big picture implementation ideas. As this is my first project in a lower level language and also specifically C, it was helpful in pointing out my dumb mistakes and providing areas where I should go research to learn more and improve.

**None of the code was written directly by AI, all the code was still validated and thought through by me**
