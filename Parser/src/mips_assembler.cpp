#include "mips_assembler.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <vector> // <-- Was missing

MipsAssembler::MipsAssembler() {
    // Correct, standard MIPS register-to-number mapping
    registerMap["$zero"] = 0; registerMap["$at"] = 1;
    registerMap["$v0"] = 2; registerMap["$v1"] = 3;
    registerMap["$a0"] = 4; registerMap["$a1"] = 5; registerMap["$a2"] = 6; registerMap["$a3"] = 7;
    registerMap["$t0"] = 8; registerMap["$t1"] = 9; registerMap["$t2"] = 10; registerMap["$t3"] = 11;
    registerMap["$t4"] = 12; registerMap["$t5"] = 13; registerMap["$t6"] = 14; registerMap["$t7"] = 15;
    registerMap["$s0"] = 16; registerMap["$s1"] = 17; registerMap["$s2"] = 18; registerMap["$s3"] = 19;
    registerMap["$s4"] = 20; registerMap["$s5"] = 21; registerMap["$s6"] = 22; registerMap["$s7"] = 23;
    registerMap["$t8"] = 24; registerMap["$t9"] = 25;
    registerMap["$k0"] = 26; registerMap["$k1"] = 27;
    registerMap["$gp"] = 28; registerMap["$sp"] = 29;
    registerMap["$fp"] = 30; registerMap["$ra"] = 31;
}

// --- FIXED: Return type is now std::vector<uint32_t> ---
std::vector<uint32_t> MipsAssembler::instructionToMachineCode(const std::string& line, uint32_t current_address) {
    std::string processed_line = line;
    // Replace commas and parentheses for easier parsing
    std::replace(processed_line.begin(), processed_line.end(), ',', ' ');
    std::replace(processed_line.begin(), processed_line.end(), '(', ' ');
    std::replace(processed_line.begin(), processed_line.end(), ')', ' ');
    
    std::stringstream ss(processed_line);
    std::string mnemonic;
    ss >> mnemonic;

    // --- R-Type Instructions ---
    if (mnemonic == "add" || mnemonic == "sub" || mnemonic == "and" || mnemonic == "or" || mnemonic == "xor" || mnemonic == "nor" || mnemonic == "slt" || mnemonic == "sltu" || mnemonic == "addu") {
        std::string rd_str, rs_str, rt_str;
        ss >> rd_str >> rs_str >> rt_str;

        // --- HACK to fix generator's addu/addiu bug ---
        if (mnemonic == "addu") {
            try {
                // Check if the 3rd operand is a number, not a register
                int imm = std::stoi(rt_str);
                // This is 'addu rd, rs, 0', treat as 'addiu rd, rs, 0'
                uint32_t opcode = 0x09; // addiu
                return {static_cast<uint32_t>((opcode << 26) | (registerMap.at(rs_str) << 21) | (registerMap.at(rd_str) << 16) | (imm & 0xFFFF))};
            } catch (const std::invalid_argument&) {
                // It's a real register, proceed as normal addu
            }
        }
        // --- End Hack ---

        uint8_t rd = registerMap.at(rd_str);
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        uint32_t funct = 0;
        if (mnemonic == "add") funct = 0x20; else if (mnemonic == "sub") funct = 0x22;
        else if (mnemonic == "and") funct = 0x24; else if (mnemonic == "or") funct = 0x25;
        else if (mnemonic == "xor") funct = 0x26; else if (mnemonic == "nor") funct = 0x27;
        else if (mnemonic == "slt") funct = 0x2A; else if (mnemonic == "sltu") funct = 0x2B;
        else if (mnemonic == "addu") funct = 0x21; 
        return {static_cast<uint32_t>((0x00 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (0 << 6) | funct)};
    }
    
    // --- FIXED: R3000 2-operand mul/div ---
    if (mnemonic == "mult" || mnemonic == "div") {
        std::string rs_str, rt_str;
        ss >> rs_str >> rt_str; // Only 2 operands from generator
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        uint32_t funct = (mnemonic == "mult") ? 0x18 : 0x1A; // R3000 funct codes
        return {static_cast<uint32_t>((0x00 << 26) | (rs << 21) | (rt << 16) | (0 << 11) | (0 << 6) | funct)};
    }
    // --- ADDED: mflo ---
    if (mnemonic == "mflo") {
        std::string rd_str;
        ss >> rd_str;
        uint8_t rd = registerMap.at(rd_str);
        return {static_cast<uint32_t>((0x00 << 26) | (0 << 21) | (0 << 16) | (rd << 11) | (0 << 6) | 0x12)};
    }
    // --- ADDED: nop ---
    if (mnemonic == "nop") {
        return {0x00000000};
    }
    // --- ADDED: jalr ---
    if (mnemonic == "jalr") {
        std::string rs_str;
        ss >> rs_str;
        uint8_t rs = registerMap.at(rs_str);
        return {static_cast<uint32_t>((0x00 << 26) | (rs << 21) | (0 << 16) | (31 << 11) | (0 << 6) | 0x09)};
    }

    if (mnemonic == "jr") {
        std::string rs_str;
        ss >> rs_str;
        return {static_cast<uint32_t>((registerMap.at(rs_str) << 21) | 0x08)};
    }
    if (mnemonic == "syscall") {
        return {0x0C};
    }
    if (mnemonic == "move") { // Pseudo-instruction
        std::string rd_str, rs_str;
        ss >> rd_str >> rs_str;
        return {static_cast<uint32_t>((registerMap.at(rs_str) << 21) | (0 << 16) | (registerMap.at(rd_str) << 11) | 0x21)}; // addu
    }

    // --- I-Type Instructions ---
    if (mnemonic == "addi" || mnemonic == "addiu" || mnemonic == "andi" || mnemonic == "ori" || mnemonic == "xori" || mnemonic == "slti" || mnemonic == "sltiu") {
        std::string rt_str, rs_str;
        int imm;
        ss >> rt_str >> rs_str >> imm;
        uint32_t opcode = 0;
        if (mnemonic == "addi") opcode = 0x08; else if (mnemonic == "addiu") opcode = 0x09;
        else if (mnemonic == "andi") opcode = 0x0C; else if (mnemonic == "ori") opcode = 0x0D;
        else if (mnemonic == "xori") opcode = 0x0E; else if (mnemonic == "slti") opcode = 0x0A;
        else if (mnemonic == "sltiu") opcode = 0x0B;
        return {static_cast<uint32_t>((opcode << 26) | (registerMap.at(rs_str) << 21) | (registerMap.at(rt_str) << 16) | (imm & 0xFFFF))};
    }
    
    // --- ADDED: li (pseudo-instruction) ---
    if (mnemonic == "li") {
        std::string rt_str;
        int imm;
        ss >> rt_str >> imm;
        // Translates to: addiu rt, $zero, imm
        uint32_t opcode = 0x09; // addiu
        return {static_cast<uint32_t>((opcode << 26) | (0 << 21) | (registerMap.at(rt_str) << 16) | (imm & 0xFFFF))};
    }

    // --- FIXED: Added lb, sb and comma handling ---
    if (mnemonic == "lw" || mnemonic == "sw" || mnemonic == "lb" || mnemonic == "sb" || mnemonic == "lui") {
        std::string rt_str, operand1, operand2;
        ss >> rt_str >> operand1 >> operand2;
        
        if (mnemonic == "lui") {
            int imm = std::stoi(operand1);
            return {static_cast<uint32_t>((0x0F << 26) | (0 << 21) | (registerMap.at(rt_str) << 16) | (imm & 0xFFFF))};
        }
        
        // This handles 'lw $t6, 0($t5)'
        int offset = std::stoi(operand1);
        std::string base_reg = operand2;

        uint32_t opcode = 0;
        if (mnemonic == "lw") opcode = 0x23;
        else if (mnemonic == "sw") opcode = 0x2B;
        else if (mnemonic == "lb") opcode = 0x20;
        else if (mnemonic == "sb") opcode = 0x28;
        return {static_cast<uint32_t>((opcode << 26) | (registerMap.at(base_reg) << 21) | (registerMap.at(rt_str) << 16) | (offset & 0xFFFF))};
    }
    
    // --- ADDED: la (pseudo-instruction) ---
    if (mnemonic == "la") {
        std::string rt_str, label;
        ss >> rt_str >> label;
        if (symbolTable.find(label) == symbolTable.end()) {
            throw std::runtime_error("Undefined label: " + label);
        }
        uint32_t address = symbolTable.at(label);
        uint16_t upper = (address >> 16) & 0xFFFF;
        uint16_t lower = address & 0xFFFF;

        uint8_t rt = registerMap.at(rt_str);
        uint32_t lui = static_cast<uint32_t>((0x0F << 26) | (0 << 21) | (rt << 16) | upper);
        uint32_t ori = static_cast<uint32_t>((0x0D << 26) | (rt << 21) | (rt << 16) | lower);
        
        if (lower == 0) return {lui}; // Optimization
        return {lui, ori};
    }
    
    if (mnemonic == "beq" || mnemonic == "bne" || mnemonic == "beqz") {
        std::string rs_str, rt_str, label;
        uint8_t rs, rt = 0;
        if (mnemonic == "beqz") {
            ss >> rs_str >> label;
            rs = registerMap.at(rs_str);
        } else {
            ss >> rs_str >> rt_str >> label;
            rs = registerMap.at(rs_str);
            rt = registerMap.at(rt_str);
        }
        if (symbolTable.find(label) == symbolTable.end()) {
            throw std::runtime_error("Undefined label: " + label);
        }
        // Branch offset is relative to the *next* instruction (PC+4)
        int32_t offset = (symbolTable.at(label) - (current_address + 4)) / 4;
        uint32_t opcode = (mnemonic == "bne") ? 0x05 : 0x04;
        return {static_cast<uint32_t>((opcode << 26) | (rs << 21) | (rt << 16) | (offset & 0xFFFF))};
    }

    // --- ADDED: seq (pseudo-instruction) ---
    if (mnemonic == "seq") {
        std::string rd_str, rs_str, rt_str;
        ss >> rd_str >> rs_str >> rt_str;
        uint8_t rd = registerMap.at(rd_str);
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        
        // xor rd, rs, rt
        uint32_t xor_instr = static_cast<uint32_t>((0x00 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (0 << 6) | 0x26);
        // sltiu rd, rd, 1
        uint32_t sltiu_instr = static_cast<uint32_t>((0x0B << 26) | (rd << 21) | (rd << 16) | 1);
        return {xor_instr, sltiu_instr};
    }

    // --- J-Type Instructions ---
    if (mnemonic == "j" || mnemonic == "jal") {
        std::string label;
        ss >> label;
        if (symbolTable.find(label) == symbolTable.end()) {
            throw std::runtime_error("Undefined label: " + label);
        }
        uint32_t address = symbolTable.at(label);
        uint32_t opcode = (mnemonic == "j") ? 0x02 : 0x03;
        // J-type target is (address / 4), masked
        return {static_cast<uint32_t>((opcode << 26) | ((address & 0x0FFFFFFF) >> 2))};
    }

    throw std::runtime_error("Unknown MIPS mnemonic: " + mnemonic);
}


void MipsAssembler::assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename) {
    // --- First Pass: Build Symbol Table ---
    uint32_t current_address = 0;
    for (const auto& line : assembly_lines) {
        if (line.empty()) continue;
        // --- FIXED: Skip all leading whitespace ---
        size_t first_char = line.find_first_not_of(" \t\n\r");
        if (first_char == std::string::npos || line[first_char] == '#' || line[first_char] == '.') continue;

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string label = line.substr(first_char, colon_pos - first_char);
            symbolTable[label] = current_address;
        } else {
            // --- FIXED: Correctly count addresses for pseudo-instructions ---
            try {
                std::string clean_line = line.substr(first_char);
                std::replace(clean_line.begin(), clean_line.end(), ',', ' ');
                std::stringstream ss(clean_line);
                std::string mnemonic;
                ss >> mnemonic;

                if (mnemonic == "seq" || mnemonic == "la") {
                    current_address += 8; // 2 instructions
                } else if (mnemonic == "move" || mnemonic == "li") {
                    current_address += 4; // 1 instruction
                } else {
                    current_address += 4; // 1 instruction
                }
            } catch (const std::exception& e) {
                 current_address += 4; // Default to 1 instruction on error
            }
        }
    }

    // --- Second Pass: Generate Machine Code ---
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open machine code output file: " + output_filename);
    }
    outfile << std::hex << std::setfill('0');

    current_address = 0;
    for (const auto& line : assembly_lines) {
        if (line.empty()) continue;
        // --- FIXED: Skip all leading whitespace ---
        size_t first_char = line.find_first_not_of(" \t\n\r");
        if (first_char == std::string::npos || line[first_char] == '#' || line[first_char] == '.' || line.find(':') != std::string::npos) {
            continue;
        }

        std::string clean_line = line.substr(first_char);
        try {
            std::vector<uint32_t> machine_codes = instructionToMachineCode(clean_line, current_address);
            
            for (uint32_t machine_code : machine_codes) {
                outfile << std::setw(8) << machine_code << std::endl;
                current_address += 4;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error assembling line: " << line << "\n" << e.what() << std::endl;
            throw; // Re-throw to stop assembly
        }
    }
}