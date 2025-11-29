#include "parser.hpp"
#include "mips_generator.hpp"
#include "symbol_table.hpp"
#include "vm_simulator.hpp"
#include "mips_assembler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map> // <-- ADDED for symbol map

// Helper to convert two hex chars (e.g., '4', 'F') to a single byte (0x4F)
uint8_t hex_to_byte(char hi, char lo) {
    auto char_to_val = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw std::runtime_error("Invalid hex character in input file.");
    };
    return (char_to_val(hi) << 4) | char_to_val(lo);
}

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

// Helper functions (unchanged)
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


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }

    std::vector<uint8_t> all_bytes;
    try {
        // ... (Hex file reading logic is correct and remains unchanged) ...
        const std::string filename = argv[1];
        std::ifstream file(filename); // Open as text
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open text file: " + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            size_t comment_pos = line.find("//");
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }
            char hi = 0;
            for (char c : line) {
                if (c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F') {
                    if (hi == 0) {
                        hi = c;
                    } else {
                        all_bytes.push_back(hex_to_byte(hi, c));
                        hi = 0;
                    }
                }
            }
            if (hi != 0) {
                 throw std::runtime_error("Found an odd number of hex digits on a line.");
            }
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error reading or parsing hex file: " << e.what() << std::endl;
        return 1;
    }

    try {
        // ... (Header parsing logic remains unchanged) ...
        if (all_bytes.size() < 20) {
             throw std::runtime_error("File is too small to contain a valid header.");
        }
        uint32_t code_section_size = read_le32(all_bytes, 4);
        uint32_t data_section_size = read_le32(all_bytes, 8);
        uint32_t symbol_table_size = read_le32(all_bytes, 12);
        std::cout << "--- Header Info ---" << std::endl;
        std::cout << "Code Section Size: " << code_section_size << " bytes" << std::endl;
        std::cout << "Data Section Size: " << data_section_size << " bytes" << std::endl;
        std::cout << "Symbol Table Size: " << symbol_table_size << " bytes" << std::endl;
        size_t current_offset = 20;
        std::vector<uint8_t> code_bytes(all_bytes.begin() + current_offset, all_bytes.begin() + current_offset + code_section_size);
        current_offset += code_section_size;
        std::vector<uint8_t> data_bytes(all_bytes.begin() + current_offset, all_bytes.begin() + current_offset + data_section_size);
        current_offset += data_section_size;
        std::vector<uint8_t> symbol_table_bytes(all_bytes.begin() + current_offset, all_bytes.begin() + current_offset + symbol_table_size);
        
        // ... (Symbol table parsing logic remains unchanged) ...
        std::cout << "\n--- Symbol Table Section (" << symbol_table_size << " bytes) ---" << std::endl;
        std::vector<SymbolEntry> symbol_table;
        size_t st_offset = 0;
        uint32_t symbol_count = read_le32(symbol_table_bytes, st_offset);
        st_offset += 4;
        std::cout << "Symbol count: " << symbol_count << std::endl;
        for (uint32_t i = 0; i < symbol_count; ++i) {
            SymbolEntry entry;
            uint32_t name_len = read_le32(symbol_table_bytes, st_offset);
            st_offset += 4;
            entry.name = std::string(reinterpret_cast<const char*>(symbol_table_bytes.data() + st_offset), name_len);
            st_offset += name_len;
            entry.type = symbol_table_bytes[st_offset++];
            entry.binding = symbol_table_bytes[st_offset++];
            entry.defined = symbol_table_bytes[st_offset++];
            entry.address = read_le32(symbol_table_bytes, st_offset);
            st_offset += 4;
            symbol_table.push_back(entry);
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

        // --- Stage 1: Parsing ---
        Parser parser(code_bytes); 
        parser.parse();
        const auto& instructions = parser.getInstructions(); // Get original list
        std::cout << "\n--- Intermediate Representation ---" << std::endl;
        parser.printInstructions();

        
        // --- NEW: Pre-processing Step (Your Idea) ---
        
        // 1. Create a map of symbols by their byte address
        std::map<uint32_t, std::vector<SymbolEntry>> symbols_by_address;
        for (const auto& sym : symbol_table) {
            if (sym.defined) { // Only care about defined symbols
                symbols_by_address[sym.address].push_back(sym);
            }
        }

        std::vector<Instruction> processed_instructions; // This is the new list
        uint32_t current_bytes = 0;
        
        // 2. Loop through original instructions and insert labels
        for (const auto& instr : instructions) {
            
            // Check if any symbols exist at the *current* byte offset
            if (symbols_by_address.count(current_bytes)) {
                for (const auto& sym : symbols_by_address[current_bytes]) {
                    
                    Instruction new_label_instr;
                    if (sym.name == "kik" || sym.name == "main") {
                        new_label_instr.name = "main:"; // As you specified
                    } else if (sym.binding == 1) { // 1 = GLOBAL
                        new_label_instr.name = ".global"; // As you specified
                    }
                    
                    if (!new_label_instr.name.empty()) {
                        processed_instructions.push_back(new_label_instr);
                    }
                }
            }
            
            // 3. Add the actual instruction from the parser
            processed_instructions.push_back(instr);
            
            // 4. Update the byte counter *after* processing
            //    This logic MUST match parser.cpp
            current_bytes++; // 1 for opcode
            if (instr.name == "ICONST" || instr.name == "JMP" || instr.name == "ISTORE" || instr.name == "ILOAD" || instr.name == "jmp_if_false" || instr.name == "JNZ") {
                current_bytes += 4;
            } else if (instr.name == "INVOKE") {
                current_bytes += 5; // 4-byte addr + 1-byte nArgs
            }
            // (Add other opcodes from parser.cpp as needed)
        }
        
        std::cout << "\n--- Processed Instruction List ---" << std::endl;
        for (const auto& instr : processed_instructions) {
             std::cout << instr.name;
             for(int op : instr.operands) { std::cout << " " << op; }
             std::cout << std::endl;
        }
        std::cout<<"\n";
        // --- END: Pre-processing Step ---

        
        int stack_size_max = 0;
        for(size_t i=0; i < processed_instructions.size(); i++) { // Use new list
            if(processed_instructions[i].name == "ICONST") {
                stack_size_max++;
            }
        }

        // --- Stage 2: Simulation ---
        // Pass the *new* processed list to the simulator
        // VMSimulator simulator(processed_instructions); 
        // // simulator.run(); // We can skip this if we just want to generate

        // // --- Stage 3: MIPS Generation ---
        // // Pass the *new* processed list to the generator
        MipsGenerator generator(processed_instructions); 
        std::vector<std::string> mips_assembly = generator.generate("output.s", stack_size_max, symbol_table);
        std::cout << "\n--- Generated MIPS Assembly ---" << std::endl;
        // for (const auto& line : mips_assembly) {
        //     std::cout << line;
        // }
        std::cout << "\nMIPS assembly Generated Successfully\n";
        // // (Printing loop removed, as it's in the generator now)
        
        MipsAssembler assembler;
        assembler.assemble(mips_assembly, "output.hex");
        std::cout << "\nSuccessfully generated MIPS assembly in output.s and machine code in output.hex" << std::endl;
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}