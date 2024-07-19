class SimpleVM:
    def __init__(self):
        self.stack = []

    def execute(self, instructions):
        for instruction in instructions:
            if instruction.startswith("PUSH"):
                _, value = instruction.split()
                self.stack.append(int(value))
            elif instruction == "POP":
                self.stack.pop()
            elif instruction == "ADD":
                self._add()
            elif instruction == "SUB":
                self._sub()
            elif instruction == "MUL":
                self._mul()
            elif instruction == "DIV":
                self._div()
            elif instruction == "PRINT":
                self._print()
            else:
                raise ValueError(f"Unknown instruction: {instruction}")

    def _add(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def _sub(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def _mul(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a * b)

    def _div(self):
        b = self.stack.pop()
        a = self.stack.pop()
        if b != 0:
            self.stack.append(a // b)
        else:
            raise ValueError("Division by zero")

    def _print(self):
        print(self.stack[-1])


# 测试虚拟机
instructions = [
    "PUSH 10",
    "PUSH 20",
    "ADD",
    "PRINT",  # 应该输出 30
    "PUSH 5",
    "MUL",
    "PRINT",  # 应该输出 150
    "PUSH 2",
    "DIV",
    "PRINT",  # 应该输出 75
]

vm = SimpleVM()
vm.execute(instructions)