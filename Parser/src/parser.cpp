#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

Parser::Parser(const std::string& filename) : filename(filename) {}

Parser::Parser(const std::string& hex_data, bool is_preprocessed) 
    : hex_stream(hex_data), is_stream_based(is_preprocessed) {}
// hexStringToBytes and parseHeader remain the same...

// NEW helper function to determine instruction length
int Parser::getOperandSize(uint8_t opcode) {
    if (format == BytecodeFormat::KATS) {
        switch (opcode) {
            case 0x01: case 0x03: case 0x0A: case 0x0B: case 0x11: return 4;
            case 0x0C: case 0x0D: case 0x1A: return 2;
            default: return 0;
        }
    } else if (format == BytecodeFormat::OATS) {
        switch (opcode) {
            case 0x08: case 0x01: case 0x07: case 0x2B: case 0x31: return 4;
            case 0x2C: case 0x2D: case 0x3A: return 2;
            default: return 0;
        }
    }
    return 0;
}

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

    std::string magic(reinterpret_cast<const char*>(bytes.data()), 4);
    
    if (magic == "KATS") {
        format = BytecodeFormat::KATS;
        std::cout << "Detected KATS format." << std::endl;
    } else if (magic == "OATS") {
        format = BytecodeFormat::OATS;
        std::cout << "Detected OATS format." << std::endl;
    } else {
        throw std::runtime_error("Unknown magic number: " + magic);
    }

    instructionCount = bytes[4] | (bytes[5] << 8) | (bytes[6] << 16) | (bytes[7] << 24);
    std::cout << "Valid header, " << instructionCount << " instructions" << std::endl;
}

void Parser::parseInstruction(const std::vector<uint8_t>& bytes) 
{
    if (bytes.empty()) return;

    uint8_t opcode = bytes[0];
    Instruction instr;

    if (format == BytecodeFormat::KATS) {
        // KATS instruction set parsing
        switch (opcode) {
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
            // ... all other KATS opcodes from the original file
            default:
                throw std::runtime_error("Unknown KATS opcode: " + std::to_string(opcode));
        }
    }
    else if (format == BytecodeFormat::OATS) 
    {
        // OATS instruction set parsing
        switch (opcode) {
            // Base Opcodes (shifted by 0x20 for uniqueness)
            case 0x01: { // ICONST
                if (bytes.size() < 5) throw std::runtime_error("ICONST requires 4 bytes for operand");
                int val = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
                instr.name = "ICONST";
                instr.operands.push_back(val);
                break;
            }
            case 0x02: instr.name = "IADD"; break;
            case 0x08: { // INVOKE
                if (bytes.size() < 5) throw std::runtime_error("INVOKE requires 4 bytes for operands");
                int funcIdx = bytes[1] | (bytes[2] << 8);
                int nArgs   = bytes[3] | (bytes[4] << 8);
                instr.name = "INVOKE";
                instr.operands.push_back(funcIdx);
                instr.operands.push_back(nArgs);
                break;
            }
            case 0x06: instr.name = "RET"; break;
            case 0x25: instr.name = "PRINT"; break;
            case 0x26: instr.name = "POP"; break;

            // Arithmetic, Comparison, Control Flow
            case 0x03: instr.name = "ISUB"; break;
            case 0x04: instr.name = "IMUL"; break;
            case 0x29: instr.name = "ILT"; break;
            case 0x07: { // JMP
                if (bytes.size() < 5) throw std::runtime_error("JMP requires 4 bytes for operand");
                int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
                instr.name = "JMP";
                instr.operands.push_back(addr);
                break;
            }
            case 0x2B: { // JMPZ
                if (bytes.size() < 5) throw std::runtime_error("JMPZ requires 4 bytes for operand");
                int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
                instr.name = "JMPZ";
                instr.operands.push_back(addr);
                break;
            }
            case 0x2C: { // LOAD
                if (bytes.size() < 3) throw std::runtime_error("LOAD requires 2 bytes for operand");
                int idx = bytes[1] | (bytes[2] << 8);
                instr.name = "LOAD";
                instr.operands.push_back(idx);
                break;
            }
            case 0x2D: { // STORE
                if (bytes.size() < 3) throw std::runtime_error("STORE requires 2 bytes for operand");
                int idx = bytes[1] | (bytes[2] << 8);
                instr.name = "STORE";
                instr.operands.push_back(idx);
                break;
            }
            // Stack, Comparison, Logical
            case 0x05: instr.name = "IDIV"; break;
            case 0x2F: instr.name = "IEQ"; break;
            case 0x30: instr.name = "IGT"; break;
            case 0x31: { // JMPNZ
                if (bytes.size() < 5) throw std::runtime_error("JMPNZ requires 4 bytes for operand");
                int addr = bytes[1] | (bytes[2] << 8) | (bytes[3] << 16) | (bytes[4] << 24);
                instr.name = "JMPNZ";
                instr.operands.push_back(addr);
                break;
            }
            case 0x32: instr.name = "DUP"; break;
            case 0x33: instr.name = "SWAP"; break;
            case 0x34: instr.name = "NOT"; break;
            // Bitwise & System Calls
            case 0x35: instr.name = "AND"; break;
            case 0x36: instr.name = "OR"; break;
            case 0x37: instr.name = "XOR"; break;
            case 0x38: instr.name = "SHL"; break;
            case 0x39: instr.name = "SHR"; break;
            case 0x3A: { // SYSCALL
                if (bytes.size() < 3) throw std::runtime_error("SYSCALL requires 2 bytes for operand");
                int call_num = bytes[1] | (bytes[2] << 8);
                instr.name = "SYSCALL";
                instr.operands.push_back(call_num);
                break;
            }
            default:
                throw std::runtime_error("Unknown OATS opcode: " + std::to_string(opcode));
        }
    }
    else 
    {
        throw std::runtime_error("Cannot parse instruction without a valid format.");
    }

    instructions.push_back(instr);
}

const std::vector<Instruction>& Parser::getInstructions() const {
    return instructions;
}

void Parser::parse() {
    // This branch handles the old, line-by-line file format for backward compatibility.
    if (!is_stream_based) {
        std::ifstream file(filename);
        if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);

        std::string line;
        bool headerParsed = false;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            auto bytes = hexStringToBytes(line);
            if (!headerParsed) {
                parseHeader(bytes);
                headerParsed = true;
            } else {
                parseInstruction(bytes);
            }
        }
        return;
    }

    // --- NEW stream-based parsing logic ---
    // This logic handles a single, continuous string of hex characters.
    if (hex_stream.length() < 16) {
        throw std::runtime_error("Hex stream is too short for a header.");
    }

    std::string header_str = hex_stream.substr(0, 16);
    parseHeader(hexStringToBytes(header_str));

    size_t current_pos = 16;
    for (int i = 0; i < instructionCount; ++i) {
        if (current_pos + 2 > hex_stream.length()) {
            throw std::runtime_error("Unexpected end of hex stream.");
        }

        std::string opcode_str = hex_stream.substr(current_pos, 2);
        uint8_t opcode = std::stoul(opcode_str, nullptr, 16);
        
        int operand_size = getOperandSize(opcode);
        int instr_total_hex_chars = (1 + operand_size) * 2;

        if (current_pos + instr_total_hex_chars > hex_stream.length()) {
            throw std::runtime_error("Incomplete instruction in stream.");
        }

        std::string instr_str = hex_stream.substr(current_pos, instr_total_hex_chars);
        parseInstruction(hexStringToBytes(instr_str));

        current_pos += instr_total_hex_chars;
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