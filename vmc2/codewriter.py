from .ir import IRProgram, Op
from . import utils
from .codewriter_helpers import EmitHelpers

from .codewriter_helpers import (
   OP_SW, R_SP, R_T0, R_T1, R_FP, R_RA, R_V0,
    OP_ADDIU, OP_LW, FUNCT_ADDU, FUNCT_JR
)

TEXT_BASE = 0x00400000
SP_INIT   = 0x7FFFEFFC


class CodeWriter:
    def __init__(self):
        self.code = []
        self.helpers = EmitHelpers(self.code)
        self.addr_of_instr = {}
        self.words = 0

    # ------------- size calculators -------------
    def size_bootstrap(self, prog: IRProgram) -> int: return 6
    def size_prologue(self) -> int: return 5
    def size_iconst(self, v: int) -> int: return (1 if -32768 <= v <= 32767 else 2) + 2
    def size_iadd(self) -> int: return 7
    def size_invoke(self) -> int: return 2
    def size_ret(self) -> int: return 8

    # ------------ first pass (measure addresses) ------------
    def measure(self, prog: IRProgram):
        self.addr_of_instr.clear()
        words = self.size_bootstrap(prog)
        for ins in prog.code:
            if ins.index == 0:
                self.addr_of_instr[ins.index] = TEXT_BASE + words*4
                continue
            if ins.index in prog.func_entries:
                words += self.size_prologue()
            self.addr_of_instr[ins.index] = TEXT_BASE + words*4
            if ins.op == Op.ICONST: words += self.size_iconst(ins.iconst_val)
            elif ins.op == Op.IADD: words += self.size_iadd()
            elif ins.op == Op.INVOKE: words += self.size_invoke()
            elif ins.op == Op.RET: words += self.size_ret()
        self.words = words

    # ------------ emit routines ------------
    def emit_prologue(self):
        self.code.append(utils.encI(OP_ADDIU, R_SP, R_SP, (-4) & 0xFFFF))
        self.code.append(utils.encI(OP_SW, R_SP, R_RA, 0))
        self.code.append(utils.encI(OP_ADDIU, R_SP, R_SP, (-4) & 0xFFFF))
        self.code.append(utils.encI(OP_SW, R_SP, R_FP, 0))
        self.helpers.emit_move(R_FP, R_SP)

    def emit_iconst(self, v: int):
        self.helpers.emit_load_imm(R_T0, v)
        self.helpers.emit_push_from(R_T0)

    def emit_iadd(self):
        self.helpers.emit_pop_to(R_T1)
        self.helpers.emit_pop_to(R_T0)
        self.code.append(utils.encR(R_T0, R_T1, R_T0, 0, FUNCT_ADDU))
        self.helpers.emit_push_from(R_T0)

    def emit_invoke(self, target_addr):
        jtarget = (target_addr >> 2) & 0x03FFFFFF
        self.code.append(utils.encJ(0x03, jtarget))  # JAL
        self.helpers.emit_nop()

    def emit_ret(self):
        self.helpers.emit_pop_to(R_T1)
        self.code.append(utils.encI(OP_LW, R_FP, R_FP, 0))
        self.code.append(utils.encI(OP_LW, R_FP, R_RA, 4))
        self.helpers.emit_push_from(R_T1)
        self.code.append(utils.encR(R_RA, 0, 0, 0, FUNCT_JR))
        self.helpers.emit_nop()

    def emit_bootstrap(self, prog: IRProgram):
        self.helpers.emit_set_sp(SP_INIT)
        entry_addr = self.addr_of_instr[prog.entry_target]
        self.emit_invoke(entry_addr)
        self.code.append(utils.encI(OP_ADDIU, 0, R_V0, 10))
        self.code.append(0x0000000C)

    # ------------ top-level ------------
    def translate(self, prog: IRProgram, out_hex: str, out_bin: str):
        self.measure(prog)
        self.code = []
        self.helpers = EmitHelpers(self.code)

        self.emit_bootstrap(prog)
        for ins in prog.code:
            if ins.index == 0: continue
            if ins.index in prog.func_entries:
                self.emit_prologue()
            if ins.op == Op.ICONST: self.emit_iconst(ins.iconst_val)
            elif ins.op == Op.IADD: self.emit_iadd()
            elif ins.op == Op.INVOKE: self.emit_invoke(self.addr_of_instr[ins.target_idx])
            elif ins.op == Op.RET: self.emit_ret()

        with open(out_hex, "w") as fh:
            for w in self.code:
                fh.write(utils.hex32(w) + "\n")
        with open(out_bin, "wb") as fb:
            for w in self.code:
                fb.write(utils.write_u32_le_bytes(w))
