#ifndef VM_SIMULATOR_HPP
#define VM_SIMULATOR_HPP

#include "parser.hpp"
#include <vector>
#include <stack>
#include<bits/stdc++.h>
class VMSimulator {
public:
    VMSimulator(const std::vector<Instruction>& instructions);
    void run();

private:
    const std::vector<Instruction>& instructions;
    std::stack<int> vm_stack;

    void printStack() const;
};

#endif