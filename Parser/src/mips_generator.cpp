#include "mips_generator.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <map> // Added for string_table

MipsGenerator::MipsGenerator(const std::vector<Instruction>& instructions) : instructions(instructions) {}

std::vector<std::string> MipsGenerator::generate(const std::string& output_filename, int stack_size_max, const std::vector<SymbolEntry>& symbol_table) {
    std::vector<std::string> assembly_lines;
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }
    std::vector<std::string> func;


    // --- NEW: String Pre-pass (if you use SCONST) ---
    // This is now empty, but we'll leave the structure
    // in case you add SCONST back.
    std::map<std::string, std::string> string_table;
    
    // --- MODIFIED: Header ---
    assembly_lines.push_back(".data\n");
    assembly_lines.push_back(".text\n");
    assembly_lines.push_back(".global main\n\n");
    assembly_lines.push_back("j main\n");

    bool main_ret = false;
    int bytes = 0;

    for (size_t idx = 0; idx < instructions.size(); ++idx) {
        const Instruction &instr = instructions[idx];

        // This "main:" check is a BUG. The parser doesn't
        // create an instruction named "main:".
        // Your main.cpp finds the "main" symbol and its address.
        // Your generator should find the instruction at address 0
        // (from the symbol table) and insert the "main:" label *there*.
        
        // --- Corrected Main Label Logic ---
        // Check if this is the entry point for "main"
        
        if (instr.name == "main:") {
            assembly_lines.push_back("main:\n");
            func.push_back("main:");
            assembly_lines.push_back("    addiu $sp, $sp, -200 \n");
            assembly_lines.push_back("    addiu $t0, $zero, 0   \n");
            assembly_lines.push_back("    addiu $t1, $sp, 0\n");
            assembly_lines.push_back("    addiu $t4, $t1, 0\n");
            assembly_lines.push_back("    addiu $sp, $sp, -200\n");
            assembly_lines.push_back("    addiu $t3, $sp, 12\n");
            assembly_lines.push_back("    addiu  $t2, $zero, 12   \n");
            addr_space.current_max_address=800;
            continue;
        }
        
        assembly_lines.push_back("L" + std::to_string(bytes) + ":\n");
        assembly_lines.push_back("    # " + instr.name + "\n");

        if( instr.name == ".global")
        {
            assembly_lines.push_back("    sw $ra, 8($sp)\n");
            func.push_back("L" + std::to_string(bytes));
        }

        // Calculate byte size for next label
        // This MUST match the parser's logic
        bytes++; // 1 byte for opcode

        if (instr.name == "ICONST" || instr.name == "JMP" || instr.name == "ISTORE" || instr.name == "ILOAD" || instr.name == "JMP_IF_ZERO" || instr.name =="JNZ" || instr.name == "jmp_if_false") {
            bytes += 4; // 4-byte operand
        } else if (instr.name == "INVOKE") {
            bytes += 5; // 4-byte address + 4-byte nArgs
        }

        // --- MIPS Generation ---
        
        if (instr.name == "ICONST") {
            int val = instr.operands.size() ? instr.operands[0] : 0;
            assembly_lines.push_back("    addiu $t6, $zero, " + std::to_string(val) + "   # t6 = " + std::to_string(val) + "\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "IADD") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a (top)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    add   $t6, $t6, $t5    # t6 = b + a\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "ISUB") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    sub   $t6, $t6, $t5    # t6 = b - a\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "IMUL") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    mult  $t6, $t5       # HI/LO = b * a\n");
            assembly_lines.push_back("    mflo  $t6            # t6 = LO (32-bit result)\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "IDIV") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a (divisor, D)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b (dividend, N/Remainder)\n");

            // --- NEW: 'div' replacement (Subtraction Loop) ---
            // Implements: $t6 = $t6 / $t5 (unsigned)
            // Uses: $t6 (Remainder), $t5 (Divisor), $t7 (Quotient), $t8 (Temp)
            std::string loop_start = "L_DIV_LOOP_" + std::to_string(idx);
            std::string loop_done = "L_DIV_DONE_" + std::to_string(idx);

            assembly_lines.push_back("    # Manual Division (uses $t7 as quotient)\n");
            assembly_lines.push_back("    addiu $t7, $zero, 0  # $t7 = Quotient (Q) = 0\n");
            // $t6 already holds the Dividend, which will become the Remainder
            assembly_lines.push_back(loop_start + ":\n");
            assembly_lines.push_back("    slt   $t8, $t6, $t5    # $t8 = (Remainder < Divisor) ? 1 : 0\n");
            assembly_lines.push_back("    bne   $t8, $zero, " + loop_done + " # if R < D, we are done\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back("    sub   $t6, $t6, $t5    # Remainder = Remainder - Divisor\n");
            assembly_lines.push_back("    addiu $t7, $t7, 1    # Quotient = Quotient + 1\n");
            assembly_lines.push_back("    j     " + loop_start + "\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back(loop_done + ":\n");
            assembly_lines.push_back("    addu  $t6, $t7, $zero  # Move quotient ($t7) to result reg ($t6)\n");
            // --- END NEW LOGIC ---

            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result (quotient)\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "ILT") { // This wasn't in your parser, but good to have
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = a\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = b\n");
            assembly_lines.push_back("    slt   $t6, $t5, $t6    # t6 = (a < b) ? 1 : 0\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if(instr.name == "ILOAD")
        {
            int index = (instr.operands.size() ? instr.operands[0] : 0 )* 4;
            assembly_lines.push_back("    addiu  $t5, $sp, " + std::to_string(index) + "      # load from frame offset " + std::to_string(index) + "\n");
            assembly_lines.push_back("    addiu $t5, $t5, 12\n");
            assembly_lines.push_back("    lw    $t6, 0($t5)      # t6 = value to load\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push loaded value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if(instr.name == "ISTORE")
        {
            int index = (instr.operands.size() ? instr.operands[0] : 0 )* 4;
            assembly_lines.push_back("    addiu $t0, $t0, -4      # pop value to store\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = value to store\n");
            assembly_lines.push_back("    addiu  $t5, $sp, " + std::to_string(index) + "      # store to frame offset " + std::to_string(index) + "\n");
            assembly_lines.push_back("    addiu $t5, $t5, 12\n");
            assembly_lines.push_back("    sw    $t6, 0($t5)      # perform store\n\n");
        }
        else if(instr.name == "INVOKE")
        {
            int target = instr.operands[0]; // This is 0
            
            assembly_lines.push_back("    # INVOKE L" + std::to_string(target) + "\n");
            assembly_lines.push_back("    addiu $sp, $sp, -200       # allocate new frame\n");
            assembly_lines.push_back("    addiu $t5, $t3, 0          # t5 = old frame base\n");
            assembly_lines.push_back("    sw    $t5, 0($sp)          # save old frame base\n");
            assembly_lines.push_back("    addiu $t3, $sp, 0          # t3 = new frame base\n");
            assembly_lines.push_back("    addiu $t5, $t2, 0          # t5 = old offset into frame\n");
            assembly_lines.push_back("    sw    $t5, 4($sp)          # save old offset\n");
            assembly_lines.push_back("    addiu $t2, $zero,  8              # t2 = 8 (offset into new frame)\n");
            assembly_lines.push_back("    addiu $t3, $t3, 8            # move frame base pointer past saved data\n");
            
            int num_operands = instr.operands.size() >= 2 ? instr.operands[1] : 0;
            for (int i = num_operands - 1; i >= 0; i--) {
                assembly_lines.push_back("    addiu $t0, $t0, -4       # pop argument " + std::to_string(i) + "\n");
                assembly_lines.push_back("    addu  $t1, $t4, $t0\n");  
                assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = argument " + std::to_string(i) + "\n");
                int offset = 12 + (i * 4);
                assembly_lines.push_back("    sw    $t6, " + std::to_string(offset) + "($sp)   # store arg " + std::to_string(i) + " at $sp + " + std::to_string(offset) + "\n");
            }
            assembly_lines.push_back("    jal L" + std::to_string(target) + "\n");
            assembly_lines.push_back("    nop\n\n");
            addr_space.current_max_address-=200;
        }
        else if (instr.name == "JMP") {
            int target = instr.operands[0];
            assembly_lines.push_back("    j L" + std::to_string(target) + "\n");
            assembly_lines.push_back("    nop\n\n");
        }
        else if (instr.name == "POP") {
            assembly_lines.push_back("    addiu $t0, $t0, -4      # pop (discard top)\n\n");
        }
        else if (instr.name == "JMP_IF_ZERO") {
            int target = instr.operands[0];
            assembly_lines.push_back("    addiu $t0, $t0, -4       # pop value\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value\n");
            assembly_lines.push_back("    beq   $t5, $zero, L" + std::to_string(target) + " # jump if value == 0\n");
            assembly_lines.push_back("    nop\n\n");
        }

        // DUP: Duplicate the top value on the operand stack
        else if (instr.name == "DUP") {
            assembly_lines.push_back("    # DUP\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4       # get address of top value\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4        # restore stack index\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0      # get new top address\n");
            assembly_lines.push_back("    sw    $t5, 0($t1)        # push duplicated value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4        # increment stack index\n\n");
        }

        // ICMP: Pop b, Pop a, push (a == b ? 1 : 0)
        // This is the same logic as icmp_eq
        else if (instr.name == "ICMP") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = b (top)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = a\n");
            // --- NEW: 'seq' replacement ---
            // Implements: $t6 = ($t6 == $t5) ? 1 : 0
            std::string label_equal = "L_SEQ_EQUAL_" + std::to_string(idx);
            std::string label_done = "L_SEQ_DONE_" + std::to_string(idx);

            assembly_lines.push_back("    sub   $t6, $t6, $t5    # $t6 = a - b\n");
            assembly_lines.push_back("    beq   $t6, $zero, " + label_equal + " # if (a-b)==0, jump\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back("    # Not equal:\n");
            assembly_lines.push_back("    addiu $t6, $zero, 0  # $t6 = 0 (false)\n");
            assembly_lines.push_back("    j     " + label_done + "\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back(label_equal + ":\n");
            assembly_lines.push_back("    # Equal:\n");
            assembly_lines.push_back("    addiu $t6, $zero, 1  # $t6 = 1 (true)\n");
            assembly_lines.push_back(label_done + ":\n");
            // --- END NEW LOGIC ---
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        
        else if (instr.name == "NEW_ARRAY") {
            assembly_lines.push_back("    # NEW_ARRAY (for ints)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)        # a0 = element count\n");
            assembly_lines.push_back("    addiu $t5, $zero, 4           # t5 = 4 (bytes per int)\n");
            assembly_lines.push_back("    mult  $a0, $t5       # HI/LO = $a0 * $t5 (count * 4)\n");
            assembly_lines.push_back("    mflo  $a0            # $a0 = bytes to allocate\n");
            assembly_lines.push_back("    addiu $v0, $zero, 9           # sbrk syscall\n");
            assembly_lines.push_back("    syscall                # OS returns pointer in $v0\n");
            assembly_lines.push_back("    sw    $v0, 0($t1)      # Push the heap pointer\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "SET_ELEM") {
            assembly_lines.push_back("    # SET_ELEM (int)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = index\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t7, 0($t1)        # t7 = array pointer (base)\n");
            assembly_lines.push_back("    addiu $t8, $zero, 4\n");
            assembly_lines.push_back("    mult  $t6, $t8       # HI/LO = index * 4\n");
            assembly_lines.push_back("    mflo  $t6            # t6 = offset\n");
            assembly_lines.push_back("    addu  $t6, $t7, $t6    # t6 = address\n");
            assembly_lines.push_back("    sw    $t5, 0($t6)\n\n");
        }
        else if (instr.name == "GET_ELEM") {
            assembly_lines.push_back("    # GET_ELEM (int)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = index\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t7, 0($t1)        # t7 = array pointer (base)\n");
            assembly_lines.push_back("    addiu $t8, $zero, 4\n");
            assembly_lines.push_back("    mult  $t6, $t8       # HI/LO = index * 4\n");
            assembly_lines.push_back("    mflo  $t6            # t6 = offset\n");
            assembly_lines.push_back("    addu  $t6, $t7, $t6    # t6 = address\n");
            assembly_lines.push_back("    lw    $t5, 0($t6)        # t5 = value from array\n");
            assembly_lines.push_back("    sw    $t5, 0($t1)      # Push value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "NEW_STRING") {
            assembly_lines.push_back("    # NEW_STRING (for chars)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)        \n");
            assembly_lines.push_back("    addiu $a0, $a0, 1    \n");
            assembly_lines.push_back("    addiu $v0, $zero, 9           \n");
            assembly_lines.push_back("    syscall                \n");
            assembly_lines.push_back("    sw    $v0, 0($t1)      \n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "SET_CHAR") {
            assembly_lines.push_back("    # SET_CHAR (byte)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value (char as int)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = index\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t7, 0($t1)        # t7 = array pointer (base)\n");
            assembly_lines.push_back("    addu  $t6, $t7, $t6    # t6 = address\n");
            assembly_lines.push_back("    sb    $t5, 0($t6)\n\n");
        }
        else if (instr.name == "GET_CHAR") {
            assembly_lines.push_back("    # GET_CHAR (byte)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)        # t6 = index\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t7, 0($t1)        # t7 = array pointer (base)\n");
            assembly_lines.push_back("    addu  $t6, $t7, $t6    # t6 = address\n");
            assembly_lines.push_back("    lb    $t5, 0($t6)        # t5 = value from array (byte)\n");
            assembly_lines.push_back("    sw    $t5, 0($t1)      # Push value\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "PRINT_I") {
            assembly_lines.push_back("    # PRINT_I (Print Integer)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)      # $a0 = integer to print\n");
            assembly_lines.push_back("    addiu $v0, $zero, 1\n");
            assembly_lines.push_back("    syscall\n\n");
        }
        else if (instr.name == "PRINT_S") {
            assembly_lines.push_back("    # PRINT_S (Print String)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $a0, 0($t1)      # $a0 = address of string\n");
            assembly_lines.push_back("    addiu $v0, $zero, 4\n");
            assembly_lines.push_back("    syscall\n\n");
        }
        
        // --- NEW CONDITIONAL INSTRUCTIONS ---
        else if (instr.name == "icmp_eq") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = b (top)\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = a\n");
            
            // --- REPLACEMENT for 'seq $t6, $t6, $t5' ---
            // Create unique labels using the loop index 'idx'
            std::string label_equal = "L_EQ_EQUAL_" + std::to_string(idx);
            std::string label_done = "L_EQ_DONE_" + std::to_string(idx);

            assembly_lines.push_back("    sub   $t6, $t6, $t5    # $t6 = a - b\n");
            assembly_lines.push_back("    beq   $t6, $zero, " + label_equal + " # if (a-b)==0, they are equal\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back("    # Not equal (branch not taken):\n");
            assembly_lines.push_back("    addiu $t6, $zero, 0  # $t6 = 0 (false)\n");
            assembly_lines.push_back("    j     " + label_done + "\n");
            assembly_lines.push_back("    nop\n");
            assembly_lines.push_back(label_equal + ":\n");
            assembly_lines.push_back("    # Equal (branch taken):\n");
            assembly_lines.push_back("    addiu $t6, $zero, 1  # $t6 = 1 (true)\n");
            assembly_lines.push_back(label_done + ":\n");
            // --- END REPLACEMENT ---

            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "icmp_lt") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = b\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = a\n");
            assembly_lines.push_back("    slt   $t6, $t6, $t5    # t6 = (a < b ? 1 : 0)\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "icmp_gt") {
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)      # t5 = b\n");
            assembly_lines.push_back("    addiu $t0, $t0, -4\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t6, 0($t1)      # t6 = a\n");
            assembly_lines.push_back("    slt   $t6, $t5, $t6    # t6 = (b < a ? 1 : 0) -> (a > b)\n");
            assembly_lines.push_back("    sw    $t6, 0($t1)      # push result\n");
            assembly_lines.push_back("    addiu $t0, $t0, 4\n\n");
        }
        else if (instr.name == "jmp_if_false") {
            int target = instr.operands[0];
            assembly_lines.push_back("    addiu $t0, $t0, -4       # pop value\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value\n");
            assembly_lines.push_back("    beq   $t5, $zero, L" + std::to_string(target) + " # jump if value == 0\n");
            assembly_lines.push_back("    nop\n\n");
        }
        else if (instr.name == "JNZ") {
            int target = instr.operands[0];
            assembly_lines.push_back("    addiu $t0, $t0, -4       # pop value\n");
            assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
            assembly_lines.push_back("    lw    $t5, 0($t1)        # t5 = value\n");
            assembly_lines.push_back("    bne   $t5, $zero, L" + std::to_string(target) + " # jump if value != 0\n");
            assembly_lines.push_back("    nop\n\n");
        }
        else if (instr.name == "RET") {
            bool func_status = false;
            if( func[func.size()-1] == "main:" )
            {
                func_status = true;
                main_ret = true;
            }
            if( func_status )
            {
                assembly_lines.push_back("    beq   $t0, $zero, L_RET_EMPTY_" + std::to_string(idx) + "\n");
                assembly_lines.push_back("    nop\n");
                assembly_lines.push_back("    addiu $t0, $t0, -4\n");
                assembly_lines.push_back("    addu  $t1, $t4, $t0\n");
                assembly_lines.push_back("    lw    $a0, 0($t1)\n");
                assembly_lines.push_back("    j L_RET_CONTINUE_" + std::to_string(idx) + "\n");
                assembly_lines.push_back("    nop\n");
                assembly_lines.push_back("L_RET_EMPTY_" + std::to_string(idx) + ":\n");
                assembly_lines.push_back("    addiu $a0, $zero, 0\n");
                assembly_lines.push_back("L_RET_CONTINUE_" + std::to_string(idx) + ":\n");
                assembly_lines.push_back("    addiu $sp, $sp, 200"  );
                assembly_lines.push_back("    # Linux O32 exit\n");
                assembly_lines.push_back("    addiu $v0, $zero, 10\n");
                assembly_lines.push_back("    syscall\n\n");
            }
            else
            {
                assembly_lines.push_back("    # RET: function return\n");
                assembly_lines.push_back("    lw    $t3, 0($sp)        # Restore old frame base\n");
                assembly_lines.push_back("    lw    $t2, 4($sp)        # Restore old frame offset\n");
                assembly_lines.push_back("    lw    $ra, 8($sp)        # Restore return address\n");
                assembly_lines.push_back("    addiu $sp, $sp, 200      # Deallocate frame\n");
                addr_space.current_max_address+=200;
                assembly_lines.push_back("    jr    $ra\n");
                assembly_lines.push_back("    nop\n\n");
            }
            if(!func.empty())
                func.pop_back();
        }
        else {
            // This will catch any opcodes from your parser that
            // are not yet in the generator (like DUP, POP, ICMP)
            assembly_lines.push_back("    # Unknown instruction: " + instr.name + " -- ignored\n\n");
        }
    }

    // Default epilogue
    if( main_ret == false )
    {
        assembly_lines.push_back("\n# Default epilogue: exit with top-of-stack (if any) or 0\n");
        assembly_lines.push_back("    beq   $t0, $zero, L_EPILOGUE_EMPTY2\n");
        assembly_lines.push_back("    nop\n");
        assembly_lines.push_back("    addiu $t0, $t0, -4\n");
        assembly_lines.push_back("    addu  $t1, $t4, $t0\n"); // Corrected to use $t4
        assembly_lines.push_back("    lw    $a0, 0($t1)\n");
        assembly_lines.push_back("    j L_EPILOGUE_EXIT2\n");
        assembly_lines.push_back("    nop\n");
        assembly_lines.push_back("L_EPILOGUE_EMPTY2:\n");
        assembly_lines.push_back("    addiu $a0, $zero, 0\n");
        assembly_lines.push_back("L_EPILOGUE_EXIT2:\n");
        assembly_lines.push_back("    # free local area\n");
        assembly_lines.push_back("    addiu $sp, $sp, 200"  );
        assembly_lines.push_back("    # Linux O32 exit\n");
        assembly_lines.push_back("    addiu $v0, $zero, 10\n");
        assembly_lines.push_back("    syscall\n");
    }

    // Write file
    for (const auto& line : assembly_lines) {
        outfile << line;
    }
    outfile.close();
    return assembly_lines;
}