#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>

// Constructor (unchanged)
Parser::Parser(const std::vector<uint8_t>& bytes) : bytecode_bytes(bytes) {}

// read_le32 helper (unchanged)
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
            
            // --- Opcodes from array.txt ---
            case 0x09: { // ISTORE
                instr.name = "ISTORE";
                int var_index = read_le32(current_pos);
                instr.operands.push_back(var_index);
                break;
            }
            case 0x0a: { // ILOAD
                instr.name = "ILOAD";
                int var_index = read_le32(current_pos);
                instr.operands.push_back(var_index);
                break;
            }
            case 0x10: instr.name = "NEW_ARRAY"; break;
            case 0x11: instr.name = "SET_ELEM"; break;
            case 0x12: instr.name = "GET_ELEM"; break;
            case 0x30: instr.name = "PRINT_I"; break;

            // --- Conditional Opcodes ---
            case 0x20: instr.name = "icmp_eq"; break;
            case 0x21: instr.name = "icmp_lt"; break;
            case 0x22: instr.name = "icmp_gt"; break;
            case 0x23: { // jmp_if_false (Jump if 0)
                instr.name = "jmp_if_false";
                int addr = read_le32(current_pos);
                instr.operands.push_back(addr);
                break;
            }
            case 0x24: { // jnz (Jump if NOT 0)
                instr.name = "JNZ";
                int addr = read_le32(current_pos);
                instr.operands.push_back(addr);
                break;
            }
            
            // --- String/Char Opcodes (Assigned) ---
            case 0x13: instr.name = "NEW_STRING"; break;
            case 0x14: instr.name = "SET_CHAR"; break;
            case 0x15: instr.name = "GET_CHAR"; break;
            case 0x31: instr.name = "PRINT_S"; break;


            default:
                std::stringstream ss;
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)opcode;
                throw std::runtime_error("Unknown OATS opcode: " + ss.str());
        }
        instructions.push_back(instr);
    }
}

// getInstructions (unchanged)
const std::vector<Instruction>& Parser::getInstructions() const {
    return instructions;
}

// printInstructions (unchanged)
void Parser::printInstructions() const {
    for (const auto& instr : instructions) {
        std::cout << instr.name;
        for (int op : instr.operands) {
            std::cout << " " << op;
        }
        std::cout << std::endl;
    }
}