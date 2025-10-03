#include "vm_simulator.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>

VMSimulator::VMSimulator(const std::vector<Instruction>& instructions) : instructions(instructions) {
    pc = 0;
    memory.resize(256, 0); // Initialize memory with 256 slots set to zero
}

void VMSimulator::printStack() const {
    // This helper function remains the same as before
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
    std::cout << "\n--- VM Simulation Start ---\n";
    std::cout << "Initial Stack: ";
    printStack();
    std::cout << "---------------------------\n";

    while (pc < instructions.size()) {
        const auto& instr = instructions[pc];
        std::cout << "PC: " << std::setw(3) << pc << " | Executing: " << std::left << std::setw(20) << (instr.name + (instr.operands.empty() ? "" : " " + std::to_string(instr.operands[0])) );

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
        } else if (instr.name == "IDIV") {
            if (vm_stack.size() < 2) throw std::runtime_error("Stack underflow for IDIV");
            int b = vm_stack.top(); vm_stack.pop();
            if (b == 0) throw std::runtime_error("Division by zero");
            int a = vm_stack.top(); vm_stack.pop();
            vm_stack.push(a / b);
        } else if (instr.name == "POP") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for POP");
            vm_stack.pop();
        } else if (instr.name == "DUP") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for DUP");
            vm_stack.push(vm_stack.top());
        } else if (instr.name == "ICMP") {
            if (vm_stack.size() < 2) throw std::runtime_error("Stack underflow for ICMP");
            int b = vm_stack.top(); vm_stack.pop();
            int a = vm_stack.top(); vm_stack.pop();
            vm_stack.push((a == b) ? 1 : 0);
        } else if (instr.name == "LOAD") {
            int var_index = instr.operands[0];
            if (var_index >= memory.size()) throw std::runtime_error("Memory access out of bounds for LOAD");
            vm_stack.push(memory[var_index]);
        } else if (instr.name == "STORE") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for STORE");
            int val = vm_stack.top(); vm_stack.pop();
            int var_index = instr.operands[0];
            if (var_index >= memory.size()) throw std::runtime_error("Memory access out of bounds for STORE");
            memory[var_index] = val;
        } else if (instr.name == "JMP") {
            pc = instr.operands[0];
            printStack(); continue;
        } else if (instr.name == "JMP_IF_ZERO") {
            if (vm_stack.empty()) throw std::runtime_error("Stack underflow for JMP_IF_ZERO");
            int val = vm_stack.top(); vm_stack.pop();
            if (val == 0) {
                pc = instr.operands[0];
                printStack(); continue;
            }
        } else if (instr.name == "INVOKE") {
            call_stack.push(pc + 1);
            pc = instr.operands[0];
            printStack(); continue;
        } else if (instr.name == "RET") {
            if (call_stack.empty()) {
                std::cout << "--- VM Simulation End ---\n";
                return;
            }
            pc = call_stack.top();
            call_stack.pop();
            printStack(); continue;
        }

        printStack();
        pc++;
    }
    std::cout << "-------------------------\n";
    std::cout << "--- VM Simulation End ---\n";
}