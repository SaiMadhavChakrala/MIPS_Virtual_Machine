#include "parser.hpp"
#include <iostream>

int main() {
        Parser parser("program.txt");
        parser.parse();
        parser.printInstructions();
        const auto& instructions = parser.getInstructions();
       
    return 0;
}