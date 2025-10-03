#ifndef VM_SIMULATOR_HPP
#define VM_SIMULATOR_HPP

#include "parser.hpp"
#include <vector>
#include <stack>

class VMSimulator {
public:
    VMSimulator(const std::vector<Instruction>& instructions);
    void run();

private:
    void printStack() const;

    const std::vector<Instruction>& instructions;
    std::stack<int> vm_stack;

    // New members for a more complete simulation
    size_t pc; // Program Counter
    std::vector<int> memory; // For LOAD and STORE
    std::stack<size_t> call_stack; // For INVOKE and RET
};

#endif