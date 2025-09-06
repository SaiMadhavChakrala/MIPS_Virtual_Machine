#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cstdint>

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

    std::vector<uint8_t> hexStringToBytes(const std::string& hex);
    void parseHeader(const std::vector<uint8_t>& bytes);
    void parseInstruction(const std::vector<uint8_t>& bytes);
};

#endif
