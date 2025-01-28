from enum import Enum

SUPPORTED_FUNCTIONS = [
    'sin', 'cos', 'log', 'ln', 'sqrt'
]

class Type(Enum):
    Operator = 1
    Number = 2
    Variable = 3
    Left_Parenthesis = 4
    Right_Parenthesis = 5
    Function = 6

class Token:
    def __init__(self, type: Type, value):
        self.type: Type = type
        self.value = value

    def __repr__(self):
        return f'({self.type}, \'{self.value}\')'


class Node:
    def __init__(self, type: Type, value, left, right):
        self.type = type
        self.value = value
        self.left = left
        self.right = right