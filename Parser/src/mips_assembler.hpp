#ifndef MIPS_ASSEMBLER_HPP
#define MIPS_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint> // <-- Added for uint8_t, uint32_t

class MipsAssembler {
public:
    MipsAssembler();
    void assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename);

private:
    std::map<std::string, uint8_t> registerMap;
    std::map<std::string, uint32_t> symbolTable;

    std::vector<uint32_t> instructionToMachineCode(const std::string& line, uint32_t current_address);
};

#endif