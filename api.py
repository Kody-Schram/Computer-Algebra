from enum import Enum

class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value

    def __repr__(self):
        return f'({self.type}, \'{self.value}\')'

class Type(Enum):
    Operator = 1
    Number = 2
    Variable = 3
    Left_Parenthesis = 4
    Right_Parenthesis = 5
    Function = 6

SUPPORTED_FUNCTIONS = [
    'sin', 'cos', 'log', 'ln', 'sqrt'
]