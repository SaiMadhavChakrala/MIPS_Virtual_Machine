#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

Parser::Parser(const std::string& filename) : filename(filename) {}

std::vector<uint8_t> Parser::hexStringToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

void Parser::parseHeader(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < 8) throw std::runtime_error("Invalid header length");

    // Magic number "STAK" = 0x53 0x54 0x41 0x4B
    std::string magic(reinterpret_cast<const char*>(bytes.data()), 4);
    if (magic != "KATS") throw std::runtime_error("Invalid magic number: " + magic);

    instructionCount = bytes[4] | (bytes[5] << 8) | (bytes[6] << 16) | (bytes[7] << 24);
    std::cout << "Valid header, " << instructionCount << " instructions" << std::endl;
}

void Parser::parseInstruction(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) return;

    uint8_t opcode = bytes[0];
    Instruction instr;

    switch (opcode) {
        // --- Base Opcodes ---
        case 0x01: { // ICONST
            if (bytes.size() < 5) throw std::runtime_error("ICONST requires 4 bytes for operand");
            int val = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
            instr.name = "ICONST";
            instr.operands.push_back(val);
            break;
        }
        case 0x02: instr.name = "IADD"; break;
        case 0x03: { // INVOKE
            if (bytes.size() < 5) throw std::runtime_error("INVOKE requires 4 bytes for operands");
            int funcIdx = bytes[1] | (bytes[2] << 8);
            int nArgs   = bytes[3] | (bytes[4] << 8);
            instr.name = "INVOKE";
            instr.operands.push_back(funcIdx);
            instr.operands.push_back(nArgs);
            break;
        }
        case 0x04: instr.name = "RET"; break;
        case 0x05: instr.name = "PRINT"; break;
        case 0x06: instr.name = "POP"; break;

        // --- Arithmetic, Comparison, Control Flow ---
        case 0x07: instr.name = "ISUB"; break;
        case 0x08: instr.name = "IMUL"; break;
        case 0x09: instr.name = "ILT"; break;
        case 0x0A: { // JMP
            if (bytes.size() < 5) throw std::runtime_error("JMP requires 4 bytes for operand");
            int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
            instr.name = "JMP";
            instr.operands.push_back(addr);
            break;
        }
        case 0x0B: { // JMPZ
            if (bytes.size() < 5) throw std::runtime_error("JMPZ requires 4 bytes for operand");
            int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
            instr.name = "JMPZ";
            instr.operands.push_back(addr);
            break;
        }
        case 0x0C: { // LOAD
            if (bytes.size() < 3) throw std::runtime_error("LOAD requires 2 bytes for operand");
            int idx = bytes[1] | (bytes[2] << 8);
            instr.name = "LOAD";
            instr.operands.push_back(idx);
            break;
        }
        case 0x0D: { // STORE
            if (bytes.size() < 3) throw std::runtime_error("STORE requires 2 bytes for operand");
            int idx = bytes[1] | (bytes[2] << 8);
            instr.name = "STORE";
            instr.operands.push_back(idx);
            break;
        }
        // --- Stack, Comparison, Logical ---
        case 0x0E: instr.name = "IDIV"; break;
        case 0x0F: instr.name = "IEQ"; break;
        case 0x10: instr.name = "IGT"; break;
        case 0x11: { // JMPNZ
            if (bytes.size() < 5) throw std::runtime_error("JMPNZ requires 4 bytes for operand");
            int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
            instr.name = "JMPNZ";
            instr.operands.push_back(addr);
            break;
        }
        case 0x12: instr.name = "DUP"; break;
        case 0x13: instr.name = "SWAP"; break;
        case 0x14: instr.name = "NOT"; break;

        // --- NEW: Bitwise & System Calls ---
        case 0x15: instr.name = "AND"; break;
        case 0x16: instr.name = "OR"; break;
        case 0x17: instr.name = "XOR"; break;
        case 0x18: instr.name = "SHL"; break;
        case 0x19: instr.name = "SHR"; break;
        case 0x1A: { // SYSCALL
            if (bytes.size() < 3) throw std::runtime_error("SYSCALL requires 2 bytes for operand");
            int call_num = bytes[1] | (bytes[2] << 8);
            instr.name = "SYSCALL";
            instr.operands.push_back(call_num);
            break;
        }


        default:
            throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }
    instructions.push_back(instr);
}


const std::vector<Instruction>& Parser::getInstructions() const {
    return instructions;
}

void Parser::parse() {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);

    std::string line;
    bool headerParsed = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto bytes = hexStringToBytes(line);
        if (!headerParsed) {
            parseHeader(bytes);
            headerParsed = true;
        } else {
            parseInstruction(bytes);
        }
    }
}

void Parser::printInstructions() const {
    for (const auto& instr : instructions) {
        std::cout << instr.name;
        for (int op : instr.operands) {
            std::cout << " " << op;
        }
        std::cout << std::endl;
    }
}
