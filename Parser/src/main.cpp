#include "parser.hpp"
#include "mips_generator.hpp"
#include <iostream>
#include "vm_simulator.hpp"
int main() {
        Parser parser("program.txt");
        parser.parse();
        parser.printInstructions();
        const auto& instructions = parser.getInstructions();
         // --- Stage 2: Simulation ---
        VMSimulator simulator(instructions);
        simulator.run();
        MipsGenerator generator(instructions);
        generator.generate("output.s");
        std::cout << "\nSuccessfully generated MIPS assembly code in output.s" << std::endl;
    return 0;
}