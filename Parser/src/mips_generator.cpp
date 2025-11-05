#include "mips_generator.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

MipsGenerator::MipsGenerator(const std::vector<Instruction>& instructions) : instructions(instructions) {}

std::vector<std::string> MipsGenerator::generate(const std::string& output_filename, int stack_size_max, const std::vector<SymbolEntry>& symbol_table) {
    std::vector<std::string> assembly_lines;
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    // Header / data
    assembly_lines.push_back(".data\n");
    assembly_lines.push_back("newline: .asciiz \"\\n\"\n");
    assembly_lines.push_back(".text\n");
    assembly_lines.push_back(".globl main\n\n");

    // Prologue: reserve an aligned local area (2 words = 8 bytes) and init index register $t0 = 0
    assembly_lines.push_back("main:\n");
    assembly_lines.push_back("    # Prologue: allocate 8 bytes for local stack (2 slots) and init index\n");

    assembly_lines.push_back("    addiu $sp, $sp, -" + std::to_string(stack_size_max*4)  + "\n");
    assembly_lines.push_back("    move  $t0, $zero    # t0 = byte-index into local area (0..8)\n\n");

    int bytes = 0;

    // Emit labels for every instruction location so JMP Lx works.
    // We'll iterate by index and place "L<index>:" before the emitted code for that instruction.
    for (size_t idx = 0; idx < instructions.size(); ++idx) {
        const Instruction &instr = instructions[idx];

        // Emit label for this instruction index (so JMP can target instruction indices).
        assembly_lines.push_back("L" + std::to_string(bytes) + ":\n");

        // Comment showing the original instruction name (helpful for debugging)
        assembly_lines.push_back("    # " + instr.name + "\n");

        bytes+=1;
        bytes+=instr.operands.size()*4;

        // ICONST value: li to t6, push via t0 index
        if (instr.name == "ICONST") {
            int val = instr.operands.size() ? instr.operands[0] : 0;
            assembly_lines.push_back("    addiu $t6, $zero, " + std::to_string(val) + "   # t6 = " + std::to_string(val) + "\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // IADD: pop a, pop b, push (b + a)
        else if (instr.name == "IADD") {
            assembly_lines.push_back("    addiu $t0, $t0, -4     # pop a (index -= 4)\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a (top)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    add   $t6, $t6, $t5    # t6 = b + a\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // ISUB: pop a, pop b, push (b - a)
        else if (instr.name == "ISUB") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    sub   $t6, $t6, $t5    # t6 = b - a\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // IMUL: pop a, pop b, push (b * a)
        else if (instr.name == "IMUL") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    mul   $t6, $t6, $t5    # t6 = b * a\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // IDIV: pop a, pop b, push (b / a)  (integer division)
        else if (instr.name == "IDIV") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    div   $t6, $t6, $t5    # t6 = b / a  (note: pseudo-instruction; assembler may expand)\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // ILT: pop a, pop b, push (a < b ? 1 : 0)
        else if (instr.name == "ILT") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    slt   $t6, $t5, $t6    # t6 = (a < b) ? 1 : 0\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        // JMP label_index: unconditional jump to label L<label_index>
        else if (instr.name == "JMP") {
            int target = instr.operands.size() ? instr.operands[0] : 0;
            assembly_lines.push_back("    j L" + std::to_string(target) + "\n");
            assembly_lines.push_back("    nop\n\n");
        }
        // PRINT: pop value and exit with that value as exit code (Linux O32 exit).
        // NOTE: to keep things simple and safe, PRINT here behaves like "exit with value".
        else if (instr.name == "PRINT") {
            assembly_lines.push_back("    addiu $t0, $t0, -4       # pop value\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)        # a0 = value to print/return\n");
            assembly_lines.push_back("    # Linux O32 exit (use exit code to convey value)\n");
            assembly_lines.push_back("    addiu $v0, $zero, 4001\n");
            assembly_lines.push_back("    syscall\n\n");
        }
        // POP: simply decrement index to discard top
        else if (instr.name == "POP") {
            assembly_lines.push_back("    addiu $t0, $t0, -4      # pop (discard top)\n\n");
        }
        // RET: attempt to pop top-of-stack into $a0 and exit with it; if empty, exit 0.
        else if (instr.name == "RET") {
            // If stack empty (t0 == 0) we set a0=0, else pop and exit with value
            assembly_lines.push_back("    beq   $t0, $zero, L_RET_EMPTY_" + std::to_string(idx) + "\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)\n");
            assembly_lines.push_back("    j L_RET_CONTINUE_" + std::to_string(idx) + "\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back("L_RET_EMPTY_" + std::to_string(idx) + ":\n");
            assembly_lines.push_back("    addiu $a0, $zero, 0\n");
            assembly_lines.push_back("L_RET_CONTINUE_" + std::to_string(idx) + ":\n");
            assembly_lines.push_back("    # Linux O32 exit\n");
            assembly_lines.push_back("    addiu $v0, $zero, 4001\n");
            assembly_lines.push_back("    syscall\n\n");
        }
        // Unknown instruction: emit comment
        else {
            assembly_lines.push_back("    # Unknown instruction: " + instr.name + " -- ignored\n\n");
        }
    }

    // Default epilogue: if the program falls through to the end, pop top-of-stack (if any) and exit with that value,
    // otherwise exit 0. This mirrors RET behavior for a program without explicit RET.
    assembly_lines.push_back("\n# Default epilogue: exit with top-of-stack (if any) or 0\n");
    assembly_lines.push_back("    beq   $t0, $zero, L_EPILOGUE_EMPTY2\n");
    assembly_lines.push_back("    nop\n");
    assembly_lines.push_back("    addiu $t0, $t0, -4\n");
    assembly_lines.push_back("    addu  $t1, $sp, $t0\n");
    assembly_lines.push_back("    lw    $a0, 0($t1)\n");
    assembly_lines.push_back("    j L_EPILOGUE_EXIT2\n");
    assembly_lines.push_back("    nop\n");
    assembly_lines.push_back("L_EPILOGUE_EMPTY2:\n");
    assembly_lines.push_back("    addiu $a0, $zero, 0\n");
    assembly_lines.push_back("L_EPILOGUE_EXIT2:\n");
    assembly_lines.push_back("    # free local area\n");
    assembly_lines.push_back("    addiu $sp, $sp, "  + std::to_string(stack_size_max*4));
    assembly_lines.push_back("    # Linux O32 exit\n");
    assembly_lines.push_back("    addiu $v0, $zero, 4001\n");
    assembly_lines.push_back("    syscall\n");

    // Write file
    for (const auto& line : assembly_lines) {
        outfile << line;
    }
    outfile.close();
    return assembly_lines;
}
