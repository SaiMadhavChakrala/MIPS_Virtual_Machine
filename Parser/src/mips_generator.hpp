#ifndef MIPS_GENERATOR_HPP
#define MIPS_GENERATOR_HPP

#include "parser.hpp"
#include <string>
#include <vector>

class MipsGenerator {
public:
    MipsGenerator(const std::vector<Instruction>& instructions);
    void generate(const std::string& output_filename);

private:
    const std::vector<Instruction>& instructions;
};

#endif