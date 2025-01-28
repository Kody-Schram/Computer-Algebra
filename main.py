from parser import Parser

expression = input('Enter expression: ')
tokens = Parser.tokenize(expression)
print(tokens)

ast = Parser.generateAST(tokens)