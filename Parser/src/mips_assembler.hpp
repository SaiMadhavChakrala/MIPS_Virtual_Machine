#ifndef MIPS_ASSEMBLER_HPP
#define MIPS_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "iostream"
class MipsAssembler {
public:
    MipsAssembler();
    void assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename);

private:
    std::map<std::string, uint8_t> registerMap;
    uint32_t instructionToMachineCode(const std::string& line);
};

#endif