# COMPUTER ALGEBRA

*A system which parses and executes mathematical expressions and equations.*

## Installation
Release binaries for the latest build are tagged and provided. Select which binary you need based on your OS and system architecture. For macOS or Linux distibutions, ensure the following dependencies are installed on your machine.

### Dependencies
- [libyaml](https://github.com/yaml/libyaml)

### Manually Building
This project uses CMake for its build process. To build locally ensure you have CMake installed. Additionally, certain OS's such as Ubuntu will need the developer versions of the required dependencies to properly link when building, ensure you have these installed. For Windows, it is recommended to just use the provided binary, however building is still possible assuming you have MinGW and the required dependencies installed.

First, clone the repository to your local machine. Then, run the following commands within the project root directory:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-march=native" ..
cmake --build .
```

`-DCMAKE_C_FLAGS="-march=native"` is not required and shouldn't be included if you plan on distributing the binary and or developing/contributing to/on this project. `Debug` is the proper build type for development. Otherwise this may marginally improve performance on your specific machine.

### Configuration

Configuration through YAML file is supported. [Example config](config.example.yaml) provides documentation on the different configurations. Unless the path is passed in through a command line argument, the file should be located at these locations:
- Windows/macOS: ./config.yaml (Relative to executable location)
- Linux: ~/.config/algebra/config.yaml


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
