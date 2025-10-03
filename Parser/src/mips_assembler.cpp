#include "mips_assembler.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>

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

uint32_t MipsAssembler::instructionToMachineCode(const std::string& line, uint32_t current_address) {
    std::string processed_line = line;
    std::replace(processed_line.begin(), processed_line.end(), ',', ' ');
    std::stringstream ss(processed_line);
    std::string mnemonic;
    ss >> mnemonic;

    // --- R-Type Instructions ---
    if (mnemonic == "add" || mnemonic == "sub" || mnemonic == "and" || mnemonic == "or" || mnemonic == "xor" || mnemonic == "nor" || mnemonic == "slt" || mnemonic == "sltu") {
        std::string rd_str, rs_str, rt_str;
        ss >> rd_str >> rs_str >> rt_str;
        uint8_t rd = registerMap.at(rd_str);
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        uint32_t funct = 0;
        if (mnemonic == "add") funct = 0x20; else if (mnemonic == "sub") funct = 0x22;
        else if (mnemonic == "and") funct = 0x24; else if (mnemonic == "or") funct = 0x25;
        else if (mnemonic == "xor") funct = 0x26; else if (mnemonic == "nor") funct = 0x27;
        else if (mnemonic == "slt") funct = 0x2A; else if (mnemonic == "sltu") funct = 0x2B;
        return (0x00 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (0 << 6) | funct;
    }
    if (mnemonic == "jr") {
        std::string rs_str;
        ss >> rs_str;
        return (registerMap.at(rs_str) << 21) | 0x08;
    }
    if (mnemonic == "syscall") {
        return 0x0C;
    }
    if (mnemonic == "move") { // Pseudo-instruction: move rd, rs -> add rd, rs, $zero
        std::string rd_str, rs_str;
        ss >> rd_str >> rs_str;
        return (registerMap.at(rs_str) << 21) | (0 << 16) | (registerMap.at(rd_str) << 11) | 0x20;
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
        return (opcode << 26) | (registerMap.at(rs_str) << 21) | (registerMap.at(rt_str) << 16) | (imm & 0xFFFF);
    }
    if (mnemonic == "lw" || mnemonic == "sw" || mnemonic == "lui") {
        std::string rt_str, operand;
        ss >> rt_str >> operand;
        if (mnemonic == "lui") {
            return (0x0F << 26) | (0 << 21) | (registerMap.at(rt_str) << 16) | (std::stoi(operand) & 0xFFFF);
        }
        size_t open_paren = operand.find('(');
        int offset = std::stoi(operand.substr(0, open_paren));
        std::string base_reg = operand.substr(open_paren + 1, operand.find(')') - open_paren - 1);
        uint32_t opcode = (mnemonic == "lw") ? 0x23 : 0x2B;
        return (opcode << 26) | (registerMap.at(base_reg) << 21) | (registerMap.at(rt_str) << 16) | (offset & 0xFFFF);
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
        int32_t offset = (symbolTable.at(label) - (current_address + 4)) / 4;
        uint32_t opcode = (mnemonic == "bne") ? 0x05 : 0x04; // beq and beqz share opcode 4
        return (opcode << 26) | (rs << 21) | (rt << 16) | (offset & 0xFFFF);
    }

    // --- J-Type Instructions ---
    if (mnemonic == "j" || mnemonic == "jal") {
        std::string label;
        ss >> label;
        uint32_t address = symbolTable.at(label);
        uint32_t opcode = (mnemonic == "j") ? 0x02 : 0x03;
        return (opcode << 26) | ((address & 0x0FFFFFFF) >> 2);
    }

    return 0x00000000; // Return NOP for any unknown instruction
}

void MipsAssembler::assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename) {
    // --- First Pass: Build Symbol Table ---
    uint32_t current_address = 0;
    for (const auto& line : assembly_lines) {
        if (line.empty()) continue;
        size_t first_char = line.find_first_not_of(" \t");
        if (first_char == std::string::npos || line[first_char] == '#' || line[first_char] == '.') continue;

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string label = line.substr(first_char, colon_pos - first_char);
            symbolTable[label] = current_address;
        } else {
            current_address += 4;
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
        size_t first_char = line.find_first_not_of(" \t");
        if (first_char == std::string::npos || line[first_char] == '#' || line[first_char] == '.' || line.find(':') != std::string::npos) {
            continue;
        }

        uint32_t machine_code = instructionToMachineCode(line.substr(first_char), current_address);
        outfile << std::setw(8) << machine_code << std::endl;
        current_address += 4;
    }
}