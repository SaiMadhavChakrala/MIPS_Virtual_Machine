#ifndef MIPS_ASSEMBLER_HPP
#define MIPS_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <fstream> // Required for std::ofstream

class MipsAssembler {
public:
    MipsAssembler();
    void assemble(const std::vector<std::string>& assembly_lines, const std::string& output_filename);

private:
    std::map<std::string, uint8_t> registerMap;
    std::map<std::string, uint32_t> symbolTable;

    // Writes a minimal, valid ELF header required by QEMU
    void writeElfHeader(std::ofstream& outfile, uint32_t entry_point, uint32_t program_size);
    uint32_t instructionToMachineCode(const std::string& line, uint32_t current_address);
};

#endif