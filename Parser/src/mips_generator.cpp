#include "mips_generator.hpp"
#include <fstream>
#include <stdexcept>

MipsGenerator::MipsGenerator(const std::vector<Instruction>& instructions) : instructions(instructions) {}

void MipsGenerator::generate(const std::string& output_filename) {
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    // MIPS file header
    outfile << ".data\n";
    outfile << "newline: .asciiz \"\\n\"\n";
    outfile << ".text\n";
    outfile << ".globl main\n\n";
    outfile << "main:\n";

    // Translate each instruction
    for (const auto& instr : instructions) {
        outfile << "    # " << instr.name << "\n";
        if (instr.name == "ICONST") {
            outfile << "    li $t0, " << instr.operands[0] << "\n";
            outfile << "    sw $t0, 0($sp)\n";
            outfile << "    addiu $sp, $sp, -4\n";
        } else if (instr.name == "IADD") {
            outfile << "    lw $t0, 4($sp)\n";
            outfile << "    lw $t1, 8($sp)\n";
            outfile << "    add $t0, $t0, $t1\n";
            outfile << "    sw $t0, 8($sp)\n";
            outfile << "    addiu $sp, $sp, 4\n";
        } else if (instr.name == "PRINT") {
            outfile << "    lw $a0, 4($sp)\n";
            outfile << "    li $v0, 1\n";
            outfile << "    syscall\n";
            // Print a newline for better formatting
            outfile << "    la $a0, newline\n";
            outfile << "    li $v0, 4\n";
            outfile << "    syscall\n";
            outfile << "    addiu $sp, $sp, 4\n"; // Pop the value
        } else if (instr.name == "POP") {
            outfile << "    addiu $sp, $sp, 4\n";
        } else if (instr.name == "RET") {
            // In this simple case, RET will exit the program
            outfile << "    li $v0, 10\n";
            outfile << "    syscall\n";
        }
        // NOTE: INVOKE is not implemented in this simple generator
    }

    // Default exit if no RET is found
    outfile << "\n    # Default exit\n";
    outfile << "    li $v0, 10\n";
    outfile << "    syscall\n";
}