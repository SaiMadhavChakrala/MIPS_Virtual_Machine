#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cstdint>

// Enum to identify the bytecode format
enum class BytecodeFormat {
    UNKNOWN,
    KATS,
    OATS
};

struct Instruction {
    std::string name;
    std::vector<int> operands;
};

class Parser {
public:
    Parser(const std::string& filename);
    void parse();
    void printInstructions() const;
    const std::vector<Instruction>& getInstructions() const;

private:
    std::string filename;
    std::vector<Instruction> instructions;
    int instructionCount = 0;
    BytecodeFormat format = BytecodeFormat::UNKNOWN; // Track the current format

    std::vector<uint8_t> hexStringToBytes(const std::string& hex);
    void parseHeader(const std::vector<uint8_t>& bytes);
    void parseInstruction(const std::vector<uint8_t>& bytes);
};

#endif