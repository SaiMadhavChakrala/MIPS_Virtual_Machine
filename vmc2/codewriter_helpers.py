from . import utils

# registers
R_ZERO=0; R_AT=1; R_V0=2; R_V1=3; R_A0=4; R_A1=5; R_A2=6; R_A3=7
R_T0=8; R_T1=9; R_T2=10; R_T3=11; R_T4=12; R_T5=13; R_T6=14; R_T7=15
R_S0=16; R_S1=17; R_S2=18; R_S3=19; R_S4=20; R_S5=21; R_S6=22; R_S7=23
R_T8=24; R_T9=25; R_K0=26; R_K1=27; R_GP=28; R_SP=29; R_FP=30; R_RA=31

# opcodes / functs
OP_J   = 0x02
OP_JAL = 0x03
OP_ADDIU = 0x09
OP_ORI = 0x0D
OP_LUI = 0x0F
OP_LW  = 0x23
OP_SW  = 0x2B
FUNCT_ADDU = 0x21
FUNCT_JR   = 0x08
FUNCT_SLL  = 0x00


class EmitHelpers:
    def __init__(self, code):
        self.code = code

    def emit_set_sp(self, sp_value: int):
        hi = (sp_value >> 16) & 0xFFFF
        lo = sp_value & 0xFFFF
        self.code.append(utils.encI(OP_LUI, R_ZERO, R_SP, hi))
        self.code.append(utils.encI(OP_ORI, R_SP, R_SP, lo))

    def emit_load_imm(self, rd: int, imm: int):
        if -32768 <= imm <= 32767:
            self.code.append(utils.encI(OP_ADDIU, R_ZERO, rd, imm & 0xFFFF))
        else:
            hi = (imm >> 16) & 0xFFFF
            lo = imm & 0xFFFF
            self.code.append(utils.encI(OP_LUI, R_ZERO, rd, hi))
            self.code.append(utils.encI(OP_ORI, rd, rd, lo))

    def emit_move(self, rd: int, rs: int):
        self.code.append(utils.encR(rs, R_ZERO, rd, 0, FUNCT_ADDU))

    def emit_push_from(self, rs: int):
        self.code.append(utils.encI(OP_ADDIU, R_SP, R_SP, (-4) & 0xFFFF))
        self.code.append(utils.encI(OP_SW, R_SP, rs, 0))

    def emit_pop_to(self, rd: int):
        self.code.append(utils.encI(OP_LW, R_SP, rd, 0))
        self.code.append(utils.encI(OP_ADDIU, R_SP, R_SP, 4))

    def emit_nop(self):
        self.code.append(utils.encR(0,0,0,0,FUNCT_SLL))
