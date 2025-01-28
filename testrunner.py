from math_parser import Parser

with open('tests/tokenizer_tests.txt', 'r') as file:
    for expr in file.readlines():
        print(Parser.tokenize(expr))