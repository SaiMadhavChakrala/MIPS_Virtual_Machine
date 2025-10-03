#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cstdint>

enum class BytecodeFormat { UNKNOWN, OATS };

struct Instruction {
    std::string name;
    std::vector<int> operands;
};

class Parser {
public:
    // New constructor for raw byte-based parsing
    Parser(const std::vector<uint8_t>& bytes); 

    void parse();
    void printInstructions() const;
    const std::vector<Instruction>& getInstructions() const;

private:
    const std::vector<uint8_t>& bytecode_bytes; // Use a const reference to avoid copying
    std::vector<Instruction> instructions;

    // Helper to read a 4-byte little-endian integer and advance the position
    int read_le32(size_t& pos);
};

#endif