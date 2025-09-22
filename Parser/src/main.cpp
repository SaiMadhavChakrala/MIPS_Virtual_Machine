#include "parser.hpp"
#include "mips_generator.hpp"
#include "vm_simulator.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // Required for std::remove_if

// Preprocesses the input file into a single, clean hex string
std::string preprocess_input(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + filename);
    }
    
    std::string content, line;
    while (std::getline(file, line)) {
        // Remove comments before appending
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        content += line;
    }

    // Remove all whitespace (spaces, newlines, tabs, etc.)
    content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());
    return content;
}

int main() {
    try {
        // --- Stage 0: Preprocessing ---
        std::string hex_data = preprocess_input("program.txt");
        
        // --- Stage 1: Parsing ---
        // Use the stream-based constructor (you'll need to add this to parser.hpp)
        Parser parser(hex_data, true); 
        parser.parse();
        const auto& instructions = parser.getInstructions();
        std::cout << "--- Intermediate Representation ---" << std::endl;
        parser.printInstructions();

        // --- Stage 2: Simulation ---
        VMSimulator simulator(instructions);
        simulator.run();

        // --- Stage 3: MIPS Generation ---
        MipsGenerator generator(instructions);
        generator.generate("output.s");
        std::cout << "\nSuccessfully generated MIPS assembly in output.s" << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}