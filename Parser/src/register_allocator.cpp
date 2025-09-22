#include "register_allocator.hpp"


RegisterAllocator::RegisterAllocator() {
    // Pool of available temporary registers ($t0-$t9)
    available_registers = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};
}

// Get a free register from the pool
std::string RegisterAllocator::acquire() {
    if (available_registers.empty()) {
        throw std::runtime_error("Compiler error: Out of temporary registers!");
    }
    std::string reg = available_registers.back();
    available_registers.pop_back();
    return reg;
}

// Return a register to the pool when it's no longer needed
void RegisterAllocator::release(const std::string& reg) {
    available_registers.push_back(reg);
}

