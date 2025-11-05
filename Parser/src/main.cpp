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
#include <iomanip>
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

// Helper functions to format symbol table data
std::string getSymbolTypeString(uint8_t type) {
    switch (type) {
        case 0: return "TEXT";
        case 1: return "DATA";
        default: return "UNKNOWN";
    }
}

std::string getSymbolBindingString(uint8_t binding) {
    switch (binding) {
        case 0: return "LOCAL";
        case 1: return "GLOBAL";
        default: return "UNKNOWN";
    }
}

std::string getSymbolDefinedString(uint8_t defined) {
    return (defined == 1) ? "true" : "false";
}

// --- NEW ---
// Structure to hold a single symbol table entry

// --- END NEW ---


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

        // --- Stage 0: Read .o file header and ALL sections ---
        std::vector<uint8_t> header_bytes(20);
        file.read(reinterpret_cast<char*>(header_bytes.data()), 20);

        if (file.gcount() != 20) {
            throw std::runtime_error("File is too small to contain a valid header.");
        }

        // Get ALL section sizes from the header
        uint32_t code_section_size = read_le32(header_bytes, 4);
        uint32_t data_section_size = read_le32(header_bytes, 8);
        uint32_t symbol_table_size = read_le32(header_bytes, 12);
        // uint32_t relocation_table_size = read_le32(header_bytes, 16); // Read but not used

        std::cout << "--- Header Info ---" << std::endl;
        std::cout << "Code Section Size: " << code_section_size << " bytes" << std::endl;
        std::cout << "Data Section Size: " << data_section_size << " bytes" << std::endl;
        std::cout << "Symbol Table Size: " << symbol_table_size << " bytes" << std::endl;

        
        // Read the exact bytes of the code section
        std::vector<uint8_t> code_bytes(code_section_size);
        file.read(reinterpret_cast<char*>(code_bytes.data()), code_section_size);

        if (file.gcount() != code_section_size) {
            throw std::runtime_error("Incomplete code section in binary file.");
        }
        
        // Read Data Section (to skip it)
        std::vector<uint8_t> data_bytes(data_section_size);
        file.read(reinterpret_cast<char*>(data_bytes.data()), data_section_size);
        if (file.gcount() != data_section_size) {
            throw std::runtime_error("Incomplete data section in binary file.");
        }

        // Read Symbol Table Section
        std::vector<uint8_t> symbol_table_bytes(symbol_table_size);
        file.read(reinterpret_cast<char*>(symbol_table_bytes.data()), symbol_table_size);
        if (file.gcount() != symbol_table_size) {
            throw std::runtime_error("Incomplete symbol table section in binary file.");
        }
        
        // --- MODIFIED: Parse Symbol Table into a vector ---
        std::cout << "\n--- Symbol Table Section (" << symbol_table_size << " bytes) ---" << std::endl;
        std::vector<SymbolEntry> symbol_table; // --- NEW ---
        
        size_t offset = 0;
        uint32_t symbol_count = read_le32(symbol_table_bytes, offset);
        offset += 4;
        std::cout << "Symbol count: " << symbol_count << std::endl;

        for (uint32_t i = 0; i < symbol_count; ++i) {
            SymbolEntry entry; // --- NEW ---

            // Read name length
            uint32_t name_len = read_le32(symbol_table_bytes, offset);
            offset += 4;

            // Read name
            entry.name = std::string(reinterpret_cast<const char*>(symbol_table_bytes.data() + offset), name_len);
            offset += name_len;

            // Read metadata
            entry.type = symbol_table_bytes[offset++];
            entry.binding = symbol_table_bytes[offset++];
            entry.defined = symbol_table_bytes[offset++];
            entry.address = read_le32(symbol_table_bytes, offset);
            offset += 4;
            
            symbol_table.push_back(entry); // --- NEW ---
        }
        
        int symbol_index = 1;
        for (const auto& entry : symbol_table) {
            std::cout << "\n// Symbol " << symbol_index << ": \"" << entry.name << "\"" << std::endl;
            std::cout << "// Length " << entry.name.length() << ", \"" << entry.name << "\"" << std::endl;
            std::cout << "// Type=" << getSymbolTypeString(entry.type)
                      << ", Binding=" << getSymbolBindingString(entry.binding)
                      << ", Defined=" << getSymbolDefinedString(entry.defined)
                      << ", Address=" << entry.address << std::endl;
            symbol_index++;
        }


        // --- Stage 1: Parsing (unchanged) ---
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

        // --- Stage 2: Simulation (unchanged) ---
        VMSimulator simulator(instructions);
        simulator.run();

        // --- Stage 3: MIPS Generation (unchanged) ---
        MipsGenerator generator(instructions );
        std::vector<std::string> mips_assembly = generator.generate("output.s", stack_size_max, symbol_table);
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