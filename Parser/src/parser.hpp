#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cstdint>

// Enum and Instruction struct remain the same
enum class BytecodeFormat { UNKNOWN, KATS, OATS };

struct Instruction {
    std::string name;
    std::vector<int> operands;
};

class Parser {
public:
    // Old constructor for file-based parsing
    Parser(const std::string& filename); 
    
    // NEW constructor for preprocessed string-based parsing
    Parser(const std::string& hex_data, bool is_preprocessed); 

    void parse();
    void printInstructions() const;
    const std::vector<Instruction>& getInstructions() const;

private:
    std::string filename;
    std::string hex_stream; // NEW: Store the hex stream
    bool is_stream_based = false; // NEW: Flag to indicate parsing mode

    std::vector<Instruction> instructions;
    int instructionCount = 0;
    BytecodeFormat format = BytecodeFormat::UNKNOWN;

    std::vector<uint8_t> hexStringToBytes(const std::string& hex);
    void parseHeader(const std::vector<uint8_t>& bytes);
    void parseInstruction(const std::vector<uint8_t>& bytes);

    // New helper to get operand size
    int getOperandSize(uint8_t opcode); 
};

#endif