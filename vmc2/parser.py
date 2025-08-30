from .fileloader import FileLoader
from .ir import IRProgram, IRInstr, Op
from typing import List
from .fileloader import LoadedFile

class Parser:
    @staticmethod
    def parse(lf: LoadedFile) -> IRProgram:
        b = lf.bytes
        FileLoader.validate_stak(b)
        if len(b) < 8:
            raise ValueError("file too small")
        n = FileLoader.read_le32_at(b, 4)
        off = 8
        code = []
        func_entries = set()
        has_entry = False
        entry_target = 0

        for i in range(n):
            if off >= len(b):
                raise ValueError(f"truncated at instr {i}")
            ins_off = off
            opcode = b[off]
            off += 1
            instr = IRInstr(op=Op(opcode), byte_offset=ins_off, index=i)
            if opcode == 0x01:  # ICONST
                if off + 4 > len(b):
                    raise ValueError("ICONST truncated")
                v = FileLoader.read_le32_at(b, off)
                # interpret as signed 32-bit
                if v & 0x80000000:
                    v = v - (1 << 32)
                instr.iconst_val = v
                off += 4
            elif opcode == 0x02:  # IADD
                pass
            elif opcode == 0x03:  # INVOKE
                if off + 4 > len(b):
                    raise ValueError("INVOKE truncated (target)")
                target = FileLoader.read_le32_at(b, off); off += 4
                if off + 1 > len(b):
                    raise ValueError("INVOKE truncated (argc)")
                argc = b[off]; off += 1
                instr.target_idx = target
                instr.argc = argc
                func_entries.add(target)
                if i == 0:
                    has_entry = True
                    entry_target = target
            elif opcode == 0x04:  # RET
                pass
            else:
                raise ValueError(f"Unknown opcode 0x{opcode:02X} at {ins_off}")

            code.append(instr)

        if not has_entry:
            raise ValueError("Instruction 0 must be 'invoke <entry> <argc>' for bootstrap")
        return IRProgram(code=code, func_entries=func_entries, has_entry_invoke=has_entry, entry_target=entry_target)
