import sys
import os
from .fileloader import FileLoader
from .parser import Parser
from .codewriter import CodeWriter

def main(argv=None):
    if argv is None:
        argv = sys.argv
    if len(argv) != 2:
        print("Usage: python -m vmc.cli <input.vm>")
        return 1
    path = argv[1]
    if not os.path.isfile(path):
        print("Input not found:", path); return 2
    lf = FileLoader.load(path)
    prog = Parser.parse(lf)
    base = os.path.splitext(path)[0]
    out_hex = base + ".hex"
    out_bin = base + ".bin"
    cw = CodeWriter()
    cw.translate(prog, out_hex, out_bin)
    print("Wrote:", out_hex, out_bin)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
