import sys, os
from .lexer import lex_file
from .parser import parse_tokens
from .codegen_mips import CodegenMIPS

def main():
    if len(sys.argv) < 2:
        print("Usage: python -m vmc.cli <file.vm>")
        sys.exit(1)

    filename = sys.argv[1]
    text = open(filename).read()
    tokens = lex_file(text)
    ir = parse_tokens(tokens, module=os.path.basename(filename))

    asm = CodegenMIPS().emit(ir)

    os.makedirs("out", exist_ok=True)
    with open("out/program.s", "w") as f:
        f.write(asm)

    print("Generated out/program.s")

if __name__ == "__main__":
    main()
