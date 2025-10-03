#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>

// Constructor that takes the raw bytes of the code section
Parser::Parser(const std::vector<uint8_t>& bytes) : bytecode_bytes(bytes) {}

// Helper to read a 4-byte little-endian integer and advance the position
int Parser::read_le32(size_t& pos) {
    if (pos + 4 > bytecode_bytes.size()) {
        throw std::runtime_error("Attempted to read past instruction bounds.");
    }
    int value = 0;
    value |= static_cast<int>(bytecode_bytes[pos + 0]);
    value |= static_cast<int>(bytecode_bytes[pos + 1]) << 8;
    value |= static_cast<int>(bytecode_bytes[pos + 2]) << 16;
    value |= static_cast<int>(bytecode_bytes[pos + 3]) << 24;
    pos += 4;
    return value;
}

void Parser::parse() {
    size_t current_pos = 0;
    while (current_pos < bytecode_bytes.size()) {
        uint8_t opcode = bytecode_bytes[current_pos];
        current_pos++;

        Instruction instr;
        instr.name = ""; 

        switch (opcode) {
            case 0x01: { // ICONST
                instr.name = "ICONST";
                int val = read_le32(current_pos);
                instr.operands.push_back(val);
                break;
            }
            case 0x02: instr.name = "IADD"; break;
            case 0x03: instr.name = "ISUB"; break;
            case 0x04: instr.name = "IMUL"; break;
            case 0x05: instr.name = "IDIV"; break;
            case 0x06: instr.name = "RET"; break;
            case 0x07: { // JMP
                instr.name = "JMP";
                int addr = read_le32(current_pos);
                instr.operands.push_back(addr);
                break;
            }
            case 0x08: { // INVOKE
                instr.name = "INVOKE";
                int addr = read_le32(current_pos);
                if (current_pos >= bytecode_bytes.size()) {
                    throw std::runtime_error("Incomplete INVOKE instruction.");
                }
                uint8_t nArgs = bytecode_bytes[current_pos];
                current_pos++;
                instr.operands.push_back(addr);
                instr.operands.push_back(nArgs);
                break;
            }
            case 0x09: { // JMP_IF_ZERO
                instr.name = "JMP_IF_ZERO";
                int addr = read_le32(current_pos);
                instr.operands.push_back(addr);
                break;
            }
            case 0x0a: { // STORE
                instr.name = "STORE";
                int var_index = read_le32(current_pos);
                instr.operands.push_back(var_index);
                break;
            }
            case 0x0b: { // LOAD
                instr.name = "LOAD";
                int var_index = read_le32(current_pos);
                instr.operands.push_back(var_index);
                break;
            }
            case 0x0c: instr.name = "DUP"; break;
            case 0x0d: instr.name = "POP"; break;
            case 0x0e: instr.name = "ICMP"; break;
            default:
                throw std::runtime_error("Unknown OATS opcode: 0x" + std::to_string(opcode));
        }

        instructions.push_back(instr);
    }
}

const std::vector<Instruction>& Parser::getInstructions() const {
    return instructions;
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