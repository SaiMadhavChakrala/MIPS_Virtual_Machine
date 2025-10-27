#include "parser.hpp"
#include "mips_generator.hpp"
#include "vm_simulator.hpp"
#include "mips_assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

// Helper function to read a 4-byte little-endian integer from a byte vector
uint32_t read_le32(const std::vector<uint8_t>& bytes, size_t offset) {
    if (offset + 4 > bytes.size()) {
        throw std::runtime_error("Attempted to read beyond byte vector boundaries.");
    }
    uint32_t value = 0;
    value |= static_cast<uint32_t>(bytes[offset + 0]);
    value |= static_cast<uint32_t>(bytes[offset + 1]) << 8;
    value |= static_cast<uint32_t>(bytes[offset + 2]) << 16;
    value |= static_cast<uint32_t>(bytes[offset + 3]) << 24;
    return value;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.o>" << std::endl;
        return 1;
    }

    try {
        const std::string filename = argv[1];
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open binary file: " + filename);
        }

        // --- Stage 0: Read .o file header and code section ---
        std::vector<uint8_t> header_bytes(20);
        file.read(reinterpret_cast<char*>(header_bytes.data()), 20);

        if (file.gcount() != 20) {
            throw std::runtime_error("File is too small to contain a valid header.");
        }

        // Get the code section size from the header at offset 4
        uint32_t code_section_size = read_le32(header_bytes, 4);
        
        // std::cout << "Detected Code Section Size: " << code_section_size << " bytes" << std::endl;
        
        // Read the exact bytes of the code section
        std::vector<uint8_t> code_bytes(code_section_size);
        file.read(reinterpret_cast<char*>(code_bytes.data()), code_section_size);

        if (file.gcount() != code_section_size) {
            throw std::runtime_error("Incomplete code section in binary file.");
        }
        
        // --- Stage 1: Parsing ---
        Parser parser(code_bytes); 
        parser.parse();
        const auto& instructions = parser.getInstructions();
        std::cout << "\n--- Intermediate Representation ---" << std::endl;
        parser.printInstructions();

        int stack_size_max = 0;
        for(int i=0; i < instructions.size(); i++) {
            if(instructions[i].name == "ICONST") {
                stack_size_max++;
            }
        }

        // --- Stage 2: Simulation ---
        VMSimulator simulator(instructions);
        simulator.run();

        // --- Stage 3: MIPS Generation ---
        MipsGenerator generator(instructions );
        std::vector<std::string> mips_assembly = generator.generate("output.s", stack_size_max);
        std::cout << "\nMIPS assemblyGenerated Successfully\n";
        for (const auto& line : mips_assembly) {
            std::cout << line;
        }
        
        MipsAssembler assembler;
        assembler.assemble(mips_assembly, "output.hex");
        std::cout << "\nSuccessfully generated MIPS assembly in output.s and machine code in output.bin" << std::endl;
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}