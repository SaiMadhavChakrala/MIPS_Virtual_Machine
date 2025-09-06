#include "parser.hpp"
#include "mips_generator.hpp"
#include <iostream>

int main() {
        Parser parser("program.txt");
        parser.parse();
        parser.printInstructions();
        const auto& instructions = parser.getInstructions();
        MipsGenerator generator(instructions);
        generator.generate("output.s");
        std::cout << "\nSuccessfully generated MIPS assembly code in output.s" << std::endl;
    return 0;
}