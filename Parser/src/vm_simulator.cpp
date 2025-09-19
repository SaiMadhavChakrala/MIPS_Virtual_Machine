#include "vm_simulator.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>

VMSimulator::VMSimulator(const std::vector<Instruction>& instructions) : instructions(instructions) {}

void VMSimulator::printStack() const {
    std::cout << "[ ";
    std::stack<int> temp = vm_stack;
    std::vector<int> contents;
    while (!temp.empty()) {
        contents.push_back(temp.top());
        temp.pop();
    }
    for (int i = contents.size() - 1; i >= 0; --i) {
        std::cout << contents[i] << " ";
    }
    std::cout << "<-- top ]" << std::endl;
}

void VMSimulator::run() {
    std::cout << "\n--- VM Simulation Start ---" << std::endl;
    std::cout << "Initial Stack: ";
    printStack();
    std::cout << "---------------------------" << std::endl;

    for (const auto& instr : instructions) {
        std::cout << "Executing: " << std::left << std::setw(15) << (instr.name + (instr.operands.empty() ? "" : " " + std::to_string(instr.operands[0])) );

        if (instr.name == "ICONST") {
            vm_stack.push(instr.operands[0]);
        } else if (instr.name == "IADD") {
            if (vm_stack.size() < 2) throw std::runtime_error("Stack underflow for IADD");
            int b = vm_stack.top(); vm_stack.pop();
            int a = vm_stack.top(); vm_stack.pop();
            vm_stack.push(a + b);
        } else if (instr.name == "ISUB") {
            if (vm_stack.size() < 2) throw std::runtime_error("Stack underflow for ISUB");
            int b = vm_stack.top(); vm_stack.pop();
            int a = vm_stack.top(); vm_stack.pop();
            vm_stack.push(a - b);
        } else if (instr.name == "IMUL") {
            if (vm_stack.size() < 2) throw std::runtime_error("Stack underflow for IMUL");
            int b = vm_stack.top(); vm_stack.pop();
            int a = vm_stack.top(); vm_stack.pop();
            vm_stack.push(a * b);
        } else if (instr.name == "PRINT") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for PRINT");
            std::cout << " | Output: " << vm_stack.top();
            // In a real VM this would pop, but for simple simulation we might leave it
        } else if (instr.name == "POP") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for POP");
            vm_stack.pop();
        }
        // ... (add other instructions here)

        printStack();
    }
     std::cout << "-------------------------\n--- Simulation End ---\n" << std::endl;
}