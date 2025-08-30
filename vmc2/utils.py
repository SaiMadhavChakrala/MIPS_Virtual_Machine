from typing import List
import struct

# MIPS encoders (32-bit words)
def encR(rs: int, rt: int, rd: int, shamt: int, funct: int) -> int:
    return ((0 & 0x3F) << 26) | ((rs & 0x1F) << 21) | ((rt & 0x1F) << 16) | ((rd & 0x1F) << 11) | ((shamt & 0x1F) << 6) | (funct & 0x3F)

def encI(op: int, rs: int, rt: int, imm: int) -> int:
    return ((op & 0x3F) << 26) | ((rs & 0x1F) << 21) | ((rt & 0x1F) << 16) | (imm & 0xFFFF)

def encJ(op: int, target: int) -> int:
    return ((op & 0x3F) << 26) | (target & 0x03FFFFFF)

def hex32(w: int) -> str:
    return f"{w & 0xFFFFFFFF:08X}"

def write_u32_le_bytes(w: int) -> bytes:
    return struct.pack("<I", w & 0xFFFFFFFF)
