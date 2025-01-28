from api import Token, Type, SUPPORTED_FUNCTIONS
from parserErrors import MismatchedParenetheses

class Parser:
    @staticmethod
    def tokenize(expression: str) -> [Token]:
        Parser.checkParentheses(expression)

        tokens: [Token] = []

        numBuffer = ''
        varBuffer = ''
        l = len(expression)

        for i in range(l):
            char = expression[i]

            if Parser.isOperator(char):
                tokens.append(
                    Token(Type.Operator, char)
                    )

            elif Parser.isLeftParen(char):
                tokens.append(
                    Token(Type.Left_Parenthesis, '(')
                    )

            elif Parser.isRightParen(char):
                tokens.append(
                    Token(Type.Right_Parenthesis, ')')
                    )

            elif Parser.isNumber(char):
                numBuffer += char

                if i < l - 1:
                    if not Parser.isNumber(expression[i + 1]):
                        tokens.append(
                            Token(Type.Number, float(numBuffer))
                            )
                        numBuffer = ''

            elif char.isalpha():
                varBuffer += char

                if Parser.isFunction(varBuffer):
                    tokens.append(Token(Type.Function, varBuffer))
                    varBuffer = ''

                elif i < l - 1:
                    if not expression[i + 1].isalpha():
                        tokens.append(
                            Token(Type.Variable, varBuffer)
                            )
                        varBuffer = ''

            if i >= l - 1:
                if numBuffer != '':
                    tokens.append(
                        Token(Type.Number, float(numBuffer))
                        )
                
                elif varBuffer != '':
                    tokens.append(
                        Token(Type.Variable, varBuffer)
                    )

        return tokens

    @staticmethod
    def generateAST(tokens: [Token]):
        # Ensure equal parenthesis



        precedence = {
            '(': 4,
            '^': 3,
            '*': 2,
            '/': 2,
            '+': 1,
            '-': 1
        }

        # Break into subexpressions within parenthesis
        parens = True

        while parens:
            index = -1
            for i, token in enumerate(tokens):
                if token.type == Type.Left_Parenthesis:
                    parenIndex = i

            if index == -1:
                parens = False

            else:
                foundRight = False

                for i in range(index, len(tokens)):
                    if tokens[i].type == Type.Right_Parenthesis:
                        foundRight = True
                        Parser.generateAST(tokens[index::i])

    @staticmethod
    def checkParentheses(expression: str) -> None:
        leftCount = expression.count('(')
        rightCount = expression.count(')')

        if leftCount != rightCount:
            raise MismatchedParenetheses()

    @staticmethod
    def isOperator(char):
        return ['^', '*', '/', '+', '-'].__contains__(char)

    @staticmethod
    def isNumber(char):
        return ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'].__contains__(char)

    @staticmethod
    def isLeftParen(char):
        return char == '('

    @staticmethod
    def isRightParen(char):
        return char == ')'

    @staticmethod
    def isFunction(func):
        return SUPPORTED_FUNCTIONS.__contains__(func)