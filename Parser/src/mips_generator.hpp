#ifndef MIPS_GENERATOR_HPP
#define MIPS_GENERATOR_HPP

#include "parser.hpp"
#include <string>
#include <vector>
#include "register_allocator.hpp" // Include the new header
#include "symbol_table.hpp" // Include the symbol table header
class MipsGenerator {
public:
    MipsGenerator(const std::vector<Instruction>& instructions);
    std::vector<std::string> generate(const std::string& output_filename, int stack_size_max,  const std::vector<SymbolEntry>& symbol_table);
    RegisterAllocator reg_alloc;
private:
    const std::vector<Instruction>& instructions;
};

#endif