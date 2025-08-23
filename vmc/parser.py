

from .ir import Instruction

def parse_tokens(tokens, module=""):
    instructions = []
    for parts in tokens:
        op = parts[0]
        if op == "push":
            instructions.append(Instruction(op="push", segment=parts[1], arg=int(parts[2])))
        elif op == "pop":
            instructions.append(Instruction(op="pop", segment=parts[1], arg=int(parts[2])))
        elif op in {"add", "sub", "neg", "eq", "lt", "gt", "and", "or", "not"}:
            instructions.append(Instruction(op=op))
        elif op == "label":
            instructions.append(Instruction(op="label", label=parts[1]))
        elif op == "goto":
            instructions.append(Instruction(op="goto", label=parts[1]))
        elif op == "if-goto":
            instructions.append(Instruction(op="if-goto", label=parts[1]))
        elif op == "function":
            instructions.append(Instruction(op="function", func=parts[1], arg=int(parts[2])))
        elif op == "call":
            instructions.append(Instruction(op="call", func=parts[1], arg=int(parts[2])))
        elif op == "return":
            instructions.append(Instruction(op="return"))
        else:
            raise ValueError(f"Unknown instruction: {parts}")
    return instructions
