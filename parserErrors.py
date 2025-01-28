class MismatchedParenetheses(Exception):
    def __init__(self, message="Unequal left and right parentheses"):
        self.message = message
        super().__init__(self.message)