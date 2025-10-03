#include "mips_generator.hpp"
#include <fstream>
#include <stdexcept>
MipsGenerator::MipsGenerator(const std::vector<Instruction>& instructions) : instructions(instructions) {}

std::vector<std::string> MipsGenerator::generate(const std::string& output_filename) {
    std::vector<std::string> assembly_lines;
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    // MIPS file header
    assembly_lines.push_back(".data\n");
    assembly_lines.push_back("newline: .asciiz \"\\n\"\n");
    assembly_lines.push_back(".text\n");
    assembly_lines.push_back(".globl main\n\n");
    assembly_lines.push_back("main:\n");

    // Translate each instruction
    
    for (const auto& instr : instructions) {
        assembly_lines.push_back("    # " + instr.name + "\n");
        if (instr.name == "ICONST") {
            std::string reg = reg_alloc.acquire(); 
            assembly_lines.push_back("    addiu " + reg + ", " +  "$zero, " + std::to_string(instr.operands[0]) + "\n"); //
            assembly_lines.push_back("    sw " + reg + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, -4\n"); //
            reg_alloc.release(reg);
        } 
        else if (instr.name == "IADD") {
            std::string reg1 = reg_alloc.acquire();
            std::string reg2 = reg_alloc.acquire();
            assembly_lines.push_back("    addiu $t7, $t7, 4\n"); 
            assembly_lines.push_back("    lw " + reg2 + ", 0($t7)\n"); // Pop second operand
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg1 + ", 0($t7)\n"); // Pop first operand
            assembly_lines.push_back("    add " + reg1 + ", " + reg1 + ", " + reg2 + "\n"); // reg1 = reg1 + reg2
            assembly_lines.push_back("    sw " + reg1 + ", 0($t7)\n");   // Push result
            assembly_lines.push_back("    addiu $t7, $t7, -4\n");
            reg_alloc.release(reg2);
            reg_alloc.release(reg1);
        }
        else if (instr.name == "ISUB") {
            std::string reg1 = reg_alloc.acquire();
            std::string reg2 = reg_alloc.acquire();
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg2 + ", 0($t7)\n"); // Pop b
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg1 + ", 0($t7)\n"); // Pop a
            assembly_lines.push_back("    sub " + reg1 + ", " + reg1 + ", " + reg2 + "\n"); // reg1 = a - b
            assembly_lines.push_back("    sw " + reg1 + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, -4\n");
            reg_alloc.release(reg2);
            reg_alloc.release(reg1);
        }
        else if (instr.name == "IMUL") {
            std::string reg1 = reg_alloc.acquire();
            std::string reg2 = reg_alloc.acquire();
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg2 + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg1 + ", 0($t7)\n");
            assembly_lines.push_back("    mul " + reg1 + ", " + reg1 + ", " + reg2 + "\n"); // reg1 = reg1 * reg2
            assembly_lines.push_back("    sw " + reg1 + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, -4\n");
            reg_alloc.release(reg2);
            reg_alloc.release(reg1);
        }
        else if (instr.name == "IDIV") {
            std::string reg1 = reg_alloc.acquire();
            std::string reg2 = reg_alloc.acquire();
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg2 + ", 0($t7)\n"); // Pop divisor
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg1 + ", 0($t7)\n"); // Pop dividend
            assembly_lines.push_back("    div " + reg1 + ", " + reg2 + "\n");       // Lo = dividend / divisor
            assembly_lines.push_back("    mflo " + reg1 + "\n");          // reg1 = Lo (quotient)
            assembly_lines.push_back("    sw " + reg1 + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, -4\n");
            reg_alloc.release(reg2);
            reg_alloc.release(reg1);
        }
        else if (instr.name == "ILT") {
            std::string reg1 = reg_alloc.acquire();
            std::string reg2 = reg_alloc.acquire();
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg2 + ", 0($t7)\n"); // Pop b
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");
            assembly_lines.push_back("    lw " + reg1 + ", 0($t7)\n"); // Pop a
            assembly_lines.push_back("    slt " + reg1 + ", " + reg1 + ", " + reg2 + "\n"); // reg1 = (a < b) ? 1 : 0
            assembly_lines.push_back("    sw " + reg1 + ", 0($t7)\n");
            assembly_lines.push_back("    addiu $t7, $t7, -4\n");
            reg_alloc.release(reg2);
            reg_alloc.release(reg1);
        }
        else if (instr.name == "JMP") {
            assembly_lines.push_back("    j L" + std::to_string(instr.operands[0]) + "\n"); // Unconditional jump to label
        }
        else if (instr.name == "PRINT") {
            assembly_lines.push_back("    addiu $t7, $t7, 4\n");   // Pop the value to print
            assembly_lines.push_back("    lw $a0, 0($t7)\n");      // Load value into argument register $a0
            assembly_lines.push_back("    li $v0, 1\n");         // Syscall code for printing integer
            assembly_lines.push_back("    syscall\n");
            // Print a newline for better formatting
            assembly_lines.push_back("    la $a0, newline\n");
            assembly_lines.push_back("    li $v0, 4\n");         // Syscall code for printing string
            assembly_lines.push_back("    syscall\n");
        } 
        else if (instr.name == "POP") {
            assembly_lines.push_back("    addiu $t7, $t7, 4\n"); // Discard top of stack by incrementing stack pointer
        } 
        else if (instr.name == "RET") {
            assembly_lines.push_back("    li $v0, 10\n");        // Syscall code for exit
            assembly_lines.push_back("    syscall\n");
        }
    }

    // Default exit if no RET is found
    // assembly_lines.push_back("\n    # Default exit\n");
    // assembly_lines.push_back("    li $v0, 10\n");
    // assembly_lines.push_back("    syscall\n");
    return assembly_lines;
}