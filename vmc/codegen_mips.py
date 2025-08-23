class CodegenMIPS:
    def __init__(self):
        self.output = []

    def emit(self, ir_list):
        self.output.append(".text\n.globl main\n")

        for instr in ir_list:
            if instr.op == "function":
                self.output.append(f"{instr.func}:\n")
                # prologue
                self.output.append("  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n")
                self.output.append("  addi $sp, $sp, -4\n  sw $fp, 0($sp)\n")
                self.output.append("  move $fp, $sp\n")
                # allocate locals
                if instr.arg > 0:
                    self.output.append(f"  addi $sp, $sp, -{4*instr.arg}\n")

            elif instr.op == "push" and instr.segment == "constant":
                self.output.append(f"  li $t0, {instr.arg}\n  jal __vm_push\n")

            elif instr.op == "add":
                self.output.append("  jal __vm_pop\n  move $t1, $t0\n")
                self.output.append("  jal __vm_pop\n  add $t0, $t0, $t1\n")
                self.output.append("  jal __vm_push\n")

            elif instr.op == "return":
                self.output.append("  jal __vm_pop\n  move $t1, $t0\n")
                self.output.append("  lw $fp, 0($fp)\n")  # restore old fp
                self.output.append("  lw $ra, 4($fp)\n")  # restore return addr
                self.output.append("  move $t0, $t1\n  jal __vm_push\n")
                self.output.append("  jr $ra\n")

        return "".join(self.output)
