#include "mips_assembler.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm> // Required for std::replace

MipsAssembler::MipsAssembler() {
    // Initialize MIPS register name to number mapping 
    registerMap["$zero"] = 0; registerMap["$at"] = 1;
    registerMap["$v0"] = 2; registerMap["$v1"] = 3;
    registerMap["$a0"] = 4; registerMap["$a1"] = 5; registerMap["$a2"] = 6; registerMap["$a3"] = 7;
    registerMap["$t0"] = 8; registerMap["$t1"] = 9; registerMap["$t2"] = 10; registerMap["$t3"] = 11;
    registerMap["$t4"] = 12; registerMap["$t5"] = 13; registerMap["$t6"] = 14; registerMap["$t7"] = 15;
    registerMap["$s0"] = 16; registerMap["$s1"] = 17; registerMap["$s2"] = 18; registerMap["$s3"] = 19;
    registerMap["$s4"] = 20; registerMap["$s5"] = 21; registerMap["$s6"] = 22; registerMap["$s7"] = 23;
    registerMap["$t8"] = 24; registerMap["$t9"] = 25;
    registerMap["$k0"] = 26; registerMap["$k1"] = 27;
    registerMap["$gp"] = 28; registerMap["$sp"] = 29; registerMap["$fp"] = 30; registerMap["$ra"] = 31;
}

uint32_t MipsAssembler::instructionToMachineCode(const std::string& line) {
    // --- FIX: Pre-process the line to replace commas with spaces ---
    std::string processed_line = line;
    std::replace(processed_line.begin(), processed_line.end(), ',', ' ');

    std::stringstream ss(processed_line);
    std::string mnemonic;
    ss >> mnemonic;
    
    // std::cout<<"\nHere I am, inside the instructionToMachineCode\n";
    
    // --- R-Type Instructions ---
    if (mnemonic == "add" || mnemonic == "sub" || mnemonic == "slt") {
        std::string rd_str, rs_str, rt_str;
        ss >> rd_str >> rs_str >> rt_str; // Removed comma parsing
        uint8_t rd = registerMap.at(rd_str);
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        uint32_t funct = 0;
        if (mnemonic == "add") funct = 0x20;
        if (mnemonic == "sub") funct = 0x22;
        if (mnemonic == "slt") funct = 0x2A;
        return (0x00 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | funct;
    }
    if (mnemonic == "mul") { // Special R-type syntax for mul
        std::string rd_str, rs_str, rt_str;
        ss >> rd_str >> rs_str >> rt_str; // Removed comma parsing
        uint8_t rd = registerMap.at(rd_str);
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        return (0x1C << 26) | (rs << 21) | (rt << 16) | (rd << 11) | 0x02;
    }
    if (mnemonic == "div") { // R-type: div rs, rt
        std::string rs_str, rt_str;
        ss >> rs_str >> rt_str; // Removed comma parsing
        uint8_t rs = registerMap.at(rs_str);
        uint8_t rt = registerMap.at(rt_str);
        return (0x00 << 26) | (rs << 21) | (rt << 16) | 0x1A;
    }
    if (mnemonic == "mflo") { // R-type: mflo rd
        std::string rd_str;
        ss >> rd_str;
        uint8_t rd = registerMap.at(rd_str);
        return (0x00 << 26) | (rd << 11) | 0x12;
    }
    if (mnemonic == "syscall") {
        return 0x0000000C;
    }

    // --- I-Type Instructions ---
    if (mnemonic == "addiu") {
        std::string rt_str, rs_str;
        int imm;
        ss >> rt_str >> rs_str >> imm; // Removed comma parsing
        uint8_t rt = registerMap.at(rt_str);
        uint8_t rs = registerMap.at(rs_str);
        return (0x09 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF);
    }
    if (mnemonic == "sw" || mnemonic == "lw") {
        std::string rt_str, full_operand;
        ss >> rt_str >> full_operand; // Removed comma parsing
        
        size_t open_paren = full_operand.find('(');
        size_t close_paren = full_operand.find(')');
        
        int offset = std::stoi(full_operand.substr(0, open_paren));
        std::string base_reg = full_operand.substr(open_paren + 1, close_paren - open_paren - 1);

        uint8_t rt = registerMap.at(rt_str);
        uint8_t rs = registerMap.at(base_reg);

        uint32_t opcode = (mnemonic == "sw") ? 0x2B : 0x23;
        return (opcode << 26) | (rs << 21) | (rt << 16) | (offset & 0xFFFF);
    }
    if (mnemonic == "li") {
        std::string rt_str;
        int imm;
        ss >> rt_str >> imm; // Removed comma parsing
        uint8_t rt = registerMap.at(rt_str);
        // This assembles 'li' as 'ori rt, $zero, imm'
        return (0x0D << 26) | (0 << 21) | (rt << 16) | (imm & 0xFFFF);
    }
    if (mnemonic == "la") { // A real assembler would handle labels. We will NOP this.
        return 0x00000000; // Placeholder for Load Address
    }
    
    // --- J-Type Instructions ---
    if (mnemonic == "j") { // A real assembler would resolve the label to an address.
        return 0x00000000; // Placeholder for Jump
    }

    // Return NOP for any other unimplemented instructions
    return 0x00000000;
}


void MipsAssembler::assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename) {
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open machine code output file: " + output_filename);
    }

    outfile << std::hex << std::setfill('0');

    for (const auto& line : assembly_lines) {
        // Filter out comments, labels, and directives
        if (line.empty()) continue;
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#' || line[first] == '.'){
            // std::cout<<"\n------------------------The dotted/# one-------------------------\n";
            continue;
        }
        if (line.find(':') != std::string::npos) {
            // std::cout<<"\n------------------------The colon one-----------------------------\n";
            continue;
        }
        // std::cout<<"\nAbout to call this one instructionToMachineCode\n";
        // std::cout<<line<<'\n';
        uint32_t machine_code = instructionToMachineCode(line.substr(first));
        
        // Only print if it's a real instruction. `la` and `j` are NOP'd for now.
        if (machine_code != 0 || line.find("nop") != std::string::npos) {
             outfile  << std::setw(8) << machine_code << std::endl;
        }
    }
}