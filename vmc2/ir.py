from dataclasses import dataclass
from enum import IntEnum
from typing import List, Set

class Op(IntEnum):
    ICONST = 0x01
    IADD   = 0x02
    INVOKE = 0x03
    RET    = 0x04

@dataclass
class IRInstr:
    op: Op
    iconst_val: int = 0
    target_idx: int = 0
    argc: int = 0
    byte_offset: int = 0
    index: int = 0

@dataclass
class IRProgram:
    code: List[IRInstr]
    func_entries: Set[int]
    has_entry_invoke: bool = False
    entry_target: int = 0
