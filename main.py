from parser import Parser

expression = input('Enter expression: ')
tokens = Parser().tokenize(expression)
print(tokens)